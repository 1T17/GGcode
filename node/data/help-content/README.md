# GGcode Help Content - Multi-Language Structure

## 📁 File Structure

This directory contains the help content split by language for better maintainability and performance.

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
├── ar.json               # Arabic content (RTL)
├── he.json               # Hebrew content (RTL)
├── tr.json               # Turkish content
├── pl.json               # Polish content
└── nl.json               # Dutch content
```

## 📊 File Statistics

- **Total Languages**: 15
- **Total Sections**: 10 per language
- **Average File Size**: ~13KB per language
- **Total Lines**: ~3,500 (vs 2,800 in monolithic file)

## 🔧 Benefits of This Structure

### ✅ **Maintainability**
- Each translator can work on their own file
- Changes to one language don't affect others
- Easier to review and approve changes

### ✅ **Performance**
- Only load the needed language(s)
- Smaller files load faster
- Better caching potential

### ✅ **Scalability**
- Easy to add new languages
- Easy to add new sections
- Better version control

### ✅ **Development**
- Easier testing of individual languages
- Better error isolation
- Simpler debugging

## 📝 Adding a New Language

1. **Create the language file**: `node/data/help-content/[lang].json`
2. **Update metadata**: Add language info to `metadata.json`
3. **Translate content**: Copy from English and translate all sections
4. **Test**: Verify the language loads correctly

## 📝 Adding a New Section

1. **Add to all language files**: Add the new section to each `[lang].json`
2. **Update metadata**: Add section info to `metadata.json`
3. **Test**: Verify the section appears in all languages

## 🔄 API Endpoints

- `GET /api/help?lang=[language]` - Get help content for specific language
- `GET /api/help/languages` - Get list of supported languages
- `GET /help?lang=[language]` - Render help template for specific language

## 🌐 RTL Language Support

The system automatically applies Right-to-Left (RTL) text direction for:
- **Arabic** (العربية) - `ar.json`
- **Hebrew** (עברית) - `he.json`

RTL support includes:
- Text direction: Right-to-Left
- Text alignment: Right-aligned
- Code blocks: Left-to-Right (for proper code display)

## 🚀 Migration Complete

✅ **Successfully migrated from monolithic 2,800-line file to 15 separate language files**
✅ **All 15 languages with all 10 sections working correctly**
✅ **Backend updated to use new file structure**
✅ **Performance improved with smaller, focused files**

## 📋 Section List

1. **overview** - What is GGcode?
2. **syntax** - Basic syntax and structure
3. **functions** - Mathematical functions
4. **control** - Control structures (loops, conditionals)
5. **arrays** - Array support and usage
6. **operators** - Comparison, logical, and mathematical operators
7. **constants** - Built-in mathematical constants
8. **documentation** - Commenting and documentation features
9. **features** - Special GGcode features
10. **bestPractices** - Programming best practices 