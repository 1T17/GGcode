# GGcode Test Suite

## Structure
```
tests/
├── scripts/           # Test automation scripts
├── Unity/            # C testing framework
├── test_*.c          # Unit test files
└── README.md         # This file
```

## Quick Start

**Run all tests:**
```bash
make test
```

**Security tests:**
```bash
./tests/scripts/test_security.sh
./tests/scripts/manual_security_test.sh
```

**Performance tests:**
```bash
./tests/scripts/test_performance.sh
```

**Edge case tests:**
```bash
./tests/scripts/test_edge_cases.sh
```

**Interactive mode tests:**
```bash
./tests/scripts/test_interactive.sh
```

## Test Categories

| Category | Files | Purpose |
|----------|-------|---------|
| **Unit Tests** | `test_*.c` | Core functionality testing |
| **Security** | `test_security*.sh` | Buffer overflow & security |
| **Performance** | `test_performance.sh` | Speed & memory benchmarks |
| **Edge Cases** | `test_edge_cases.sh` | Unusual input handling |
| **Interactive** | `test_interactive.sh` | CLI menu testing |

## Results
- **216 unit tests** - All core modules
- **11 security tests** - Buffer overflow protection
- **100% pass rate** - Production ready

Run `make test` for comprehensive testing.