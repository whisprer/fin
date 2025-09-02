# Developing WofLang Modules  

Modules are C++ shared libraries (DLLs) loaded dynamically.  

**Required Interface**:
```cpp
extern "C" __declspec(dllexport) void RegisterOps(WofLangAPI* api);
Steps:

Define your ops with Unicode-friendly identifiers

Use RegisterOps to expose them

Compile as DLL, place in /modules

Example:

cpp
Copy
Edit
void RegisterOps(WofLangAPI* api) {
    api->AddOp("⚡", [](Stack& s) { /* your op */ });
}
yaml
Copy
Edit

---

## FILE: `/docs/collaboration.md`

```markdown
# Contributing to WofLang

We welcome:

- New plugins/modules
- Core language improvements
- Documentation enhancements
- Philosophical/Artistic experiments

### Setup

- C++23
- CMake
- Fork, branch, PR workflow

**Philosophy**:  
WofLang is both tool and art—contributions should honor both precision and creativity.