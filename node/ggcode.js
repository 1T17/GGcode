const express = require('express');
const bodyParser = require('body-parser');
const ffi = require('ffi-napi');
const path = require('path');

const app = express();
const PORT = 3000;

// FFI Setup
const libPath = path.resolve(__dirname, 'libggcode.so');
const ggcode = ffi.Library(libPath, {
  compile_ggcode_from_string: ['string', ['pointer', 'int']]
});

function decodeHTMLEntities(str) {
  return str.replace(/&lt;/g, '<')
            .replace(/&gt;/g, '>')
            .replace(/&amp;/g, '&');
}


function stripCarriageReturns(str) {
  return str.replace(/\r/g, '');
}








function printCharCodes(label, str) {
  console.log(`\n[${label}] Character codes:`);
  for (let i = 0; i < str.length; i++) {
    const c = str[i];
    const code = c.charCodeAt(0);
    let display = c;
    if (c === '\n') display = '\\n';
    else if (c === '\r') display = '\\r';
    else if (c === '\t') display = '\\t';
    else if (c === ' ') display = '␣'; // visible space

    console.log(`  [${i}] '${display}' → ${code}`);
  }
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

  const output = ggcode.compile_ggcode_from_string(inputBuffer, 1);


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
    const output = ggcode.compile_ggcode_from_string(inputBuffer, 1);
    res.json({ success: true, output });
  } catch (err) {
    res.json({ success: false, error: err.message || 'Compilation error' });
  }
});


// Start server
app.listen(PORT, () => {
  console.log(`Server running at http://localhost:${PORT}`);
});
