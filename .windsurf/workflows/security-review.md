# Security Review Command

## Purpose
Perform comprehensive security analysis of code changes, focusing on:
- Buffer overflow vulnerabilities
- Memory safety issues
- Input validation problems
- Integer overflow/underflow
- Unsafe function usage
- Backwards compatibility impact

## Review Checklist

### Critical Security Issues
- [ ] **Buffer Overflows**: Check all sprintf/strcpy/memcpy usage for proper bounds checking
- [ ] **Memory Safety**: Verify malloc/free pairs, NULL checks, double-free prevention
- [ ] **Integer Safety**: Check for overflow in arithmetic operations and array indexing
- [ ] **Input Validation**: Ensure all external inputs are validated and sanitized
- [ ] **Unsafe Functions**: Replace sprintf/strcpy/gets/strcat with safe alternatives

### Code Quality & Compatibility
- [ ] **Functionality**: Verify no loss of existing functionality
- [ ] **Backwards Compatibility**: Ensure API changes don't break existing code
- [ ] **Error Handling**: Check that error paths are properly handled
- [ ] **Resource Management**: Verify proper cleanup in all code paths
- [ ] **Thread Safety**: Check for race conditions in multi-threaded contexts

### Performance Impact
- [ ] **Performance**: Assess any performance regressions
- [ ] **Memory Usage**: Check for memory leaks or excessive allocations
- [ ] **Algorithm Complexity**: Verify no unintended complexity increases

## Security Analysis Framework

### 1. Buffer Overflow Analysis
```cpp
// ❌ Unsafe
sprintf(buffer, "%s", input);
strcpy(dest, src);

// ✅ Safe
snprintf(buffer, sizeof(buffer), "%s", input);
strncpy(dest, src, sizeof(dest) - 1);
dest[sizeof(dest) - 1] = '\0';
```

### 2. Memory Safety Analysis
```cpp
// ❌ Unsafe
ptr = malloc(size);
*ptr = value; // No NULL check

// ✅ Safe  
ptr = malloc(size);
if (!ptr) return ERROR;
*ptr = value;
```

### 3. Integer Overflow Analysis
```cpp
// ❌ Unsafe
int result = a + b; // Could overflow
array[index + 1] = value; // Could overflow index

// ✅ Safe
if (__builtin_add_overflow(a, b, &result)) return ERROR;
if (index >= INT_MAX - 1) return ERROR;
array[index + 1] = value;
```

## Recent Changes Review Template

For each change, analyze:

1. **Security Impact**: What vulnerabilities are introduced/fixed?
2. **Functionality**: Does behavior change in any way?
3. **Compatibility**: Will existing code break?
4. **Performance**: Any performance implications?
5. **Testing**: How should this be tested?

## Example Review Output

### Change: Replace __mingw_sprintf with __mingw_snprintf
- **Security**: ✅ Fixes buffer overflow vulnerability
- **Functionality**: ✅ No change in output, same format string
- **Compatibility**: ✅ API unchanged, internal implementation only
- **Performance**: ✅ Negligible impact, same complexity
- **Testing**: Test with very large floating point values near buffer limits

### Change: Replace sscanf with strtold  
- **Security**: ✅ Better error handling, prevents undefined behavior
- **Functionality**: ⚠️ May handle edge cases differently (overflow/underflow)
- **Compatibility**: ✅ API unchanged, internal implementation only
- **Performance**: ✅ Similar performance, better error detection
- **Testing**: Test with overflow values, invalid strings, edge cases

## Automated Checks

Run these commands during review:
```bash
# Find unsafe functions
grep -r "sprintf(" --include="*.c" --include="*.cpp"
grep -r "strcpy(" --include="*.c" --include="*.cpp" 
grep -r "gets(" --include="*.c" --include="*.cpp"

# Find unchecked malloc
grep -r "malloc(" --include="*.c" --include="*.cpp" -A1 -B1

# Find potential integer overflow
grep -r "\[.*+.*\]" --include="*.c" --include="*.cpp"
```

## Severity Classification

- **Critical**: Security vulnerabilities that could lead to code execution
- **High**: Security vulnerabilities that could lead to crashes or data corruption  
- **Medium**: Security issues with limited impact
- **Low**: Code quality issues with no security impact
