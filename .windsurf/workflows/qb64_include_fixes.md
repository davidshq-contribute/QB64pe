---
description: How to fix QB64 include order and function dependency issues
---

## QB64 Include Order and Dependency Issues

When encountering compilation errors related to missing functions or labels in included files, follow this structured approach:

### Root Cause Analysis
1. **Text Substitution**: QB64's `$INCLUDE` is text substitution, not module system
2. **Implicit END**: QB64 injects END before first SUB/FUNCTION encountered
3. **Dependency Order**: Functions must be available when called, but includes may be in wrong order

### Proper Solution Pattern

#### Step 1: Identify the Issue Type
- **Missing Function**: Function called but not defined in current compilation context
- **Missing Label**: GOTO to label not in current compilation context
- **Statement between SUB/FUNCTION**: Code placement after implicit END

#### Step 2: Choose the Correct Fix Strategy

**For Missing Functions:**
1. Create forward declaration in separate `.bi` file
2. Include declaration file before calling code
3. Include implementation file after calling code
4. Follow the three-phase pattern: declarations → main code → implementations

**For Missing Labels:**
1. Replace GOTO with structured control flow (IF/ELSE, DO/LOOP)
2. Use inline error handling instead of cross-file GOTO
3. Follow the refactoring patterns in `docs/problems_encountered/qb64_goto_labels_in_included_functions.md`

**For Statement Placement Issues:**
1. Move executable code before any SUB/FUNCTION definitions
2. Use forward declarations for all needed functions
3. Separate declaration includes from implementation includes

#### Step 3: Implementation Pattern
```basic
' Phase 1: Declarations
'$INCLUDE:'dependencies_declarations.bi'

' Phase 2: Main program code
MainFunctionCall

' Phase 3: Implementations  
'$INCLUDE:'implementations.bas'
```

### Key Rules
1. **Never use cross-file GOTO labels** - refactor to structured control flow
2. **Always forward declare functions** when calling before definition
3. **Separate concerns** - declarations in .bi, implementations in .bas
4. **Test compilation** after each fix to verify structure

### Related Documentation
- [QB64 Include System](auto-including.md)
- [GOTO Label Restrictions](../problems_encountered/qb64_goto_labels_in_included_functions.md)
- [Main Program Structure](../problems_encountered/qb64_main_program_structure.md)
- [QB64 Syntax Rules](../../QB64_SYNTAX_RULES.md)
