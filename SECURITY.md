# GGcode Security Measures

## Input Validation & Limits

### File Path Security
- **Path Traversal Protection**: Blocks `../` and `..\` sequences in filenames and output paths
- **Excessive Traversal**: Prevents deep path traversal attempts (`../../../`)
- **Filename Length**: Maximum 256 characters per filename
- **Output Path Length**: Maximum 512 characters for output paths

### Input Limits
- **Maximum Files**: 1000 input files per compilation
- **Evaluation Code**: 4096 character limit for `-e/--eval` expressions
- **Interactive Input**: Sanitized to alphanumeric characters only

### Memory Safety
- **Buffer Overflow Protection**: All string operations use safe functions (`strncpy`, `snprintf`)
- **Input Bounds**: Fixed-size buffers with length validation
- **Memory Allocation**: Proper error handling for malloc/realloc failures

## Interactive Mode Security

### Input Sanitization
- Menu selection limited to valid characters (0-9, a-z, A-Z)
- Invalid characters rejected with error message
- Buffer size limited to 10 characters

### File Discovery Limits
- Maximum 1000 .ggcode files displayed in interactive menu
- Warning shown if directory contains too many files
- Memory allocation failures handled gracefully

## Command Line Security

### Argument Validation
- Length validation for all string arguments
- Path traversal detection in file arguments
- Bounds checking for array operations

### Safe String Operations
- No use of unsafe functions (`strcpy`, `strcat`, `gets`, `sprintf`)
- All string operations use bounded versions
- Null termination guaranteed

## File System Security

### Output Directory Safety
- Write permission testing before file creation
- Fallback to current directory if target inaccessible
- Temporary file creation for permission validation

### Cross-Platform Compatibility
- Windows and Unix path handling
- Platform-specific directory operations
- Safe file enumeration with bounds checking

## Error Handling

### Graceful Failures
- Clear error messages for invalid input
- Memory cleanup on error conditions
- Safe exit paths for all error scenarios

### Resource Management
- Proper memory deallocation
- File handle cleanup
- Directory handle closure

## Recommendations

### For Users
- Avoid extremely long filenames or paths
- Use relative paths when possible
- Keep evaluation expressions reasonable in size

### For Developers
- All new string operations must use safe functions
- Add bounds checking for any new input handling
- Test with malformed input and edge cases

## Security Limits Summary

| Component | Limit | Purpose |
|-----------|-------|---------|
| Filename Length | 256 chars | Prevent buffer overflow |
| Output Path | 512 chars | Safe path handling |
| Eval Code | 4096 chars | Memory management |
| Input Files | 1000 files | Resource limits |
| Interactive Input | 10 chars | Input validation |
| Menu Files | 1000 files | UI performance |

These measures protect against common vulnerabilities while maintaining usability.