const express = require('express');
const bodyParser = require('body-parser');
const ffi = require('ffi-napi');
const path = require('path');
const fs = require('fs');

const app = express();
const PORT = 6990;

// FFI Setup
const libPath = path.resolve(__dirname, 'libggcode.so');
const ggcode = ffi.Library(libPath, {
  compile_ggcode_from_string: ['pointer', ['pointer', 'int']],
  free_ggcode_string: ['void', ['pointer']]
});

function decodeHTMLEntities(str) {
  return str.replace(/&lt;/g, '<')
            .replace(/&gt;/g, '>')
            .replace(/&amp;/g, '&');
}

function stripCarriageReturns(str) {
  return str.replace(/\r/g, '');
}

// Middleware
app.use(bodyParser.urlencoded({ extended: true }));
app.use(express.json());
app.use(express.static('public'));
app.set('view engine', 'ejs');

// Routes
app.get('/', (req, res) => {
  res.render('index', { output: '', input: '' });
});

app.post('/compile', (req, res) => {
  const rawInput = req.body.ggcode || '';
  const decodedInput = decodeHTMLEntities(rawInput);
  const cleanInput = stripCarriageReturns(decodedInput);
  const inputBuffer = Buffer.from(cleanInput + '\0', 'utf8');

  const outputPtr = ggcode.compile_ggcode_from_string(inputBuffer, 1);
  let output = '';
  if (outputPtr && !outputPtr.isNull && typeof outputPtr.readCString === 'function') {
    output = outputPtr.readCString();
  } else if (outputPtr) {
    output = outputPtr.readCString ? outputPtr.readCString() : Buffer.from(outputPtr).toString();
  }
  ggcode.free_ggcode_string(outputPtr);
  if (global.gc) {
    global.gc();
  }
  res.render('index', { output, input: rawInput });
});

// Add AJAX API endpoint for compilation
app.post('/api/compile', (req, res) => {
  let code = '';
  if (req.is('application/json')) {
    code = req.body.ggcode || '';
  } else {
    code = req.body.ggcode || '';
  }
  const decodedInput = decodeHTMLEntities(code);
  const cleanInput = stripCarriageReturns(decodedInput);
  const inputBuffer = Buffer.from(cleanInput + '\0', 'utf8');

  try {
    const outputPtr = ggcode.compile_ggcode_from_string(inputBuffer, 1);
    let output = '';
    if (outputPtr && !outputPtr.isNull && typeof outputPtr.readCString === 'function') {
      output = outputPtr.readCString();
    } else if (outputPtr) {
      output = outputPtr.readCString ? outputPtr.readCString() : Buffer.from(outputPtr).toString();
    }
    ggcode.free_ggcode_string(outputPtr);
    if (global.gc) {
      global.gc();
    }
    res.json({ success: true, output });
  } catch (err) {
    res.json({ success: false, error: err.message || 'Compilation error' });
  }
});

// Examples API endpoints
app.get('/api/examples', (req, res) => {
  const examplesDir = path.join(__dirname, 'GGCODE');
  
  try {
    if (!fs.existsSync(examplesDir)) {
      return res.json({ success: false, error: 'Examples directory not found' });
    }
    
    const files = fs.readdirSync(examplesDir)
      .filter(file => file.endsWith('.ggcode'))
      .map(file => {
        const filePath = path.join(examplesDir, file);
        const content = fs.readFileSync(filePath, 'utf8');
        const lines = content.split('\n');
        const firstComment = lines.find(line => line.trim().startsWith('//'));
        const description = firstComment ? firstComment.replace('//', '').trim() : '';
        const preview = lines.slice(0, 3).join('\n').substring(0, 100) + '...';
        
        return {
          name: file,
          description: description,
          preview: preview
        };
      });
    
    res.json({ success: true, files });
  } catch (error) {
    res.json({ success: false, error: error.message });
  }
});

app.get('/api/examples/:filename', (req, res) => {
  const filename = req.params.filename;
  const filePath = path.join(__dirname, 'GGCODE', filename);
  
  try {
    if (!fs.existsSync(filePath)) {
      return res.json({ success: false, error: 'File not found' });
    }
    
    const content = fs.readFileSync(filePath, 'utf8');
    res.json({ success: true, content });
  } catch (error) {
    res.json({ success: false, error: error.message });
  }
});

// Help API endpoints
app.get('/api/help', (req, res) => {
  try {
    const language = req.query.lang || 'en'; // Default to English
    const metadataPath = path.join(__dirname, 'data', 'help-content', 'metadata.json');
    const langFilePath = path.join(__dirname, 'data', 'help-content', `${language}.json`);
    
    // Check if metadata exists
    if (!fs.existsSync(metadataPath)) {
      return res.json({ success: false, error: 'Help metadata not found' });
    }
    
    // Check if language file exists
    if (!fs.existsSync(langFilePath)) {
      return res.json({ success: false, error: `Language '${language}' not found` });
    }
    
    // Load metadata and language data
    const metadata = JSON.parse(fs.readFileSync(metadataPath, 'utf8'));
    const langData = JSON.parse(fs.readFileSync(langFilePath, 'utf8'));
    
    // Construct response in the expected format
    const helpData = {
      metadata: {
        version: metadata.version,
        lastUpdated: metadata.lastUpdated,
        supportedLanguages: metadata.supportedLanguages.map(lang => lang.code),
        defaultLanguage: metadata.defaultLanguage
      },
      sections: langData.sections
    };
    
    res.json({ success: true, data: helpData, language: language });
  } catch (error) {
    res.json({ success: false, error: 'Failed to load help content: ' + error.message });
  }
});

app.get('/api/help/languages', (req, res) => {
  try {
    const metadataPath = path.join(__dirname, 'data', 'help-content', 'metadata.json');
    
    if (!fs.existsSync(metadataPath)) {
      return res.json({ success: false, error: 'Help metadata not found' });
    }
    
    const metadata = JSON.parse(fs.readFileSync(metadataPath, 'utf8'));
    
    res.json({ 
      success: true, 
      languages: metadata.supportedLanguages,
      defaultLanguage: metadata.defaultLanguage
    });
  } catch (error) {
    res.json({ success: false, error: 'Failed to load languages: ' + error.message });
  }
});

app.get('/help', (req, res) => {
  try {
    const language = req.query.lang || 'en';
    const langFilePath = path.join(__dirname, 'data', 'help-content', `${language}.json`);
    
    if (!fs.existsSync(langFilePath)) {
      return res.status(404).send(`Language '${language}' not found`);
    }
    
    const langData = JSON.parse(fs.readFileSync(langFilePath, 'utf8'));
    res.render('help-template', { sections: langData.sections, language: language });
  } catch (error) {
    res.status(500).send('Failed to load help content: ' + error.message);
  }
});

// Start server
app.listen(PORT, () => {
console.log(`
 _____ _____           _                  _        __ _____ 
|   __|   __|___ ___ _| |___    ___ ___ _| |___ __|  |   __|
|  |  |  |  |  _| . | . | -_|  |   | . | . | -_|  |  |__   |
|_____|_____|___|___|___|___|  |_|_|___|___|___|_____|_____|                                                                                                  
Server running at:${PORT} \n`);
});
