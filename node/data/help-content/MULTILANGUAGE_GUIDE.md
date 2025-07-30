# 🌍 GGcode Multi-Language Help System Guide

## **Overview**

The GGcode help system now supports multiple languages through a modular file-based architecture. This guide explains how it works and how to add new languages.

---

## **🎯 How It Works**

### **1. File Structure**

The help content is organized in a modular structure with separate files for each language:

```
node/data/help-content/
├── metadata.json          # Language metadata and section info
├── en.json               # English content
├── es.json               # Spanish content
├── fr.json               # French content
├── de.json               # German content
├── it.json               # Italian content
├── pt.json               # Portuguese content
├── ru.json               # Russian content
├── zh.json               # Chinese content
├── ja.json               # Japanese content
├── ko.json               # Korean content
├── ar.json               # Arabic content
└── README.md             # Documentation
```

### **2. Language File Structure**

Each language file contains all sections for that language:

```json
{
  "language": "es",
  "sections": {
    "overview": {
      "id": "overview",
      "title": "¿Qué es GGcode?",
      "content": [...]
    },
    "syntax": {
      "id": "syntax",
      "title": "Sintaxis",
      "content": [...]
    }
  }
}
```

### **3. Metadata Structure**

The metadata file contains language configuration:

```json
{
  "version": "1.0",
  "supportedLanguages": [
    {
      "code": "en",
      "name": "English",
      "flag": "🇺🇸",
      "file": "en.json"
    },
    {
      "code": "es",
      "name": "Español",
      "flag": "🇪🇸",
      "file": "es.json"
    }
  ],
  "defaultLanguage": "en",
  "sections": [
    {
      "id": "overview",
      "key": "overview",
      "name": "Overview"
    }
  ]
}
```

### **2. Backend API**

The system provides these endpoints:

- **`/api/help?lang=en`** - Get help content for specific language
- **`/api/help/languages`** - Get list of supported languages
- **`/help?lang=es`** - Render help template with specific language

### **3. Frontend Integration**

- **Language Selector** - Dropdown in help modal header
- **Dynamic Loading** - Content loads based on selected language
- **Automatic Rendering** - Handles both single and multi-language structures

---

## **🚀 Adding a New Language**

### **Step 1: Create Language File**

Create a new language file: `node/data/help-content/[lang].json`

```json
{
  "language": "nl",
  "sections": {
    "overview": {
      "id": "overview",
      "title": "Wat is GGcode?",
      "content": [...]
    },
    "syntax": {
      "id": "syntax",
      "title": "Syntaxis",
      "content": [...]
    }
  }
}
```

### **Step 2: Update Metadata**

Add the new language to `node/data/help-content/metadata.json`:

```json
{
  "supportedLanguages": [
    {
      "code": "en",
      "name": "English",
      "flag": "🇺🇸",
      "file": "en.json"
    },
    {
      "code": "nl",
      "name": "Nederlands",
      "flag": "🇳🇱",
      "file": "nl.json"
    }
  ]
}
```

### **Step 3: Add Language to Frontend**

Update `node/views/helpExamples.ejs`:

```html
<select id="languageSelect" class="language-select">
  <option value="en">🇺🇸 English</option>
  <option value="es">🇪🇸 Español</option>
  <option value="fr">🇫🇷 Français</option>
  <option value="nl">🇳🇱 Nederlands</option>  <!-- New language -->
</select>
```

### **Step 4: Test the New Language**

The backend automatically detects new languages from the metadata file, so no backend changes are needed!

---

## **📝 Content Types Supported**

The system supports these content types in any language:

### **Paragraph**
```json
{
  "type": "paragraph",
  "text": "Your translated text here"
}
```

### **List**
```json
{
  "type": "list",
  "title": "Optional title",
  "items": [
    "Item 1 in target language",
    "Item 2 in target language"
  ]
}
```

### **Subsection**
```json
{
  "type": "subsection",
  "title": "Translated title",
  "description": "Translated description",
  "additionalInfo": "Optional additional info",
  "code": "Code examples (usually same across languages)"
}
```

### **Code Block**
```json
{
  "type": "code",
  "code": "let example = 'code'  // Comments can be translated"
}
```

---

## **🔧 Advanced Features**

### **1. Fallback Language**

If a section doesn't exist in the selected language, the system can fall back to the default language:

```javascript
// In renderHelpContent function
let sectionData = section;
if (section[language]) {
  sectionData = section[language];
} else if (section[defaultLanguage]) {
  sectionData = section[defaultLanguage];
}
```

### **2. Language Detection**

Automatically detect user's preferred language:

```javascript
// Get browser language
const userLanguage = navigator.language.split('-')[0];
const supportedLanguages = ['en', 'es', 'fr', 'de', 'he', 'ar'];
const defaultLanguage = supportedLanguages.includes(userLanguage) ? userLanguage : 'en';
```

### **3. RTL Language Support**

The system automatically detects and applies Right-to-Left (RTL) text direction for Arabic and Hebrew:

```css
/* RTL Support for Arabic and Hebrew */
[data-lang="ar"] .help-content,
[data-lang="he"] .help-content {
  direction: rtl;
  text-align: right;
}
```

### **3. Language Persistence**

Save user's language preference:

```javascript
// Save to localStorage
localStorage.setItem('ggcode-language', selectedLanguage);

// Load from localStorage
const savedLanguage = localStorage.getItem('ggcode-language') || 'en';
```

---

## **🌐 Translation Guidelines**

### **1. Code Examples**
- **Keep code the same** across all languages
- **Translate comments** in code examples
- **Maintain formatting** and indentation

### **2. Technical Terms**
- **Consistent terminology** within each language
- **Use standard translations** for programming terms
- **Consider cultural context** for examples

### **3. File Organization**
```
node/data/help-content/
├── metadata.json                  # Language configuration
├── en.json                        # English content
├── es.json                        # Spanish content
├── fr.json                        # French content
├── de.json                        # German content
├── it.json                        # Italian content
├── pt.json                        # Portuguese content
├── ru.json                        # Russian content
├── zh.json                        # Chinese content
├── ja.json                        # Japanese content
├── ko.json                        # Korean content
├── ar.json                        # Arabic content (RTL)
├── he.json                        # Hebrew content (RTL)
└── README.md                      # Documentation
```

---

## **🎯 Benefits of This System**

### **✅ Maintainability**
- **Separate content** from presentation
- **Easy updates** - just edit JSON files
- **Version control** - track changes per language

### **✅ Scalability**
- **Add languages** without code changes
- **Modular structure** - translate sections independently
- **Performance** - load only needed content

### **✅ User Experience**
- **Native language** support
- **Consistent interface** across languages
- **Fast switching** between languages

### **✅ Developer Experience**
- **Simple JSON** structure
- **Reusable components** - same template for all languages
- **Easy testing** - validate each language independently

---

## **🚀 Future Enhancements**

### **1. Translation Management**
- **Translation workflow** tools
- **Missing translation** detection
- **Translation memory** for consistency

### **2. Dynamic Content**
- **User-specific** help content
- **Context-aware** translations
- **Regional variations** within languages

### **3. Advanced Features**
- **Voice search** in multiple languages
- **Multilingual examples** database
- **Community translations** system

---

## **📋 Quick Start Checklist**

- [ ] Create new language file: `node/data/help-content/[lang].json`
- [ ] Add language to metadata: `node/data/help-content/metadata.json`
- [ ] Add language to frontend selector: `node/views/helpExamples.ejs`
- [ ] Translate all content sections
- [ ] Test language switching
- [ ] Validate fallback behavior
- [ ] Update documentation

---

## **🎉 Example: Adding Dutch Support**

1. **Create Dutch file**: `node/data/help-content/nl.json`
2. **Add Dutch to metadata**: Update `metadata.json`
3. **Add Dutch option** to language selector
4. **Test** Dutch content loading
5. **Verify** search works in Dutch

The system is now ready to support Dutch users with native language help content! 