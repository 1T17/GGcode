# Contributing to GGcode

Thank you for your interest in contributing to GGcode! Your help makes this project better for everyone.

## 📝 How to Contribute

- **Bug Reports & Feature Requests:**
  - Please use [GitHub Issues](https://github.com/1T17/GGcode/issues) to report bugs or suggest features.
  - Include as much detail as possible (steps to reproduce, expected/actual behavior, screenshots, etc).

- **Pull Requests:**
  1. Fork the repository and create your branch from `main`.
  2. Write clear, concise commit messages.
  3. Add tests for new features or bug fixes if possible.
  4. Ensure all tests pass (`make test` or `npm test` if applicable).
  5. Open a pull request and describe your changes.

## 🧑‍💻 Code Style

- **C code:**
  - Use 4 spaces for indentation.
  - Follow existing naming conventions.
  - Add comments for complex logic.
- **JavaScript/Node.js:**
  - Use 2 spaces for indentation.
  - Prefer `const`/`let` over `var`.
  - Use semicolons and single quotes.
- **General:**
  - Keep functions small and focused.
  - Write descriptive variable and function names.
  - Document public functions with comments.

## 🧪 Testing

- Run all tests before submitting a pull request.
- Add new tests for new features or bug fixes.
- Test both the C and Node.js sides if your change affects both.

## ✍️ Contributing GGcode Scripts & Algorithms

We welcome contributions of new GGcode scripts, toolpath algorithms, and parametric machining logic! Here’s how to make your script contributions clear, reusable, and high-quality:

### Script Style & Best Practices
- **Use clear variable names** (e.g., `radius`, `feed_rate`, `passes`).
- **Add comments** to explain the purpose of each section and any tricky logic.
- **Group related logic** into functions when possible.
- **Use `note {}` blocks** for runtime comments and debug output.
- **Prefer parametric/variable-driven code** over hardcoded values.
- **Document required input variables** at the top of your script.
- **Include a sample output or screenshot** if possible (in your pull request).

### Example GGcode Script Contribution
```ggcode
// Spiral pocketing example
let center_x = 0
let center_y = 0
let start_radius = 5
let end_radius = 20
let step = 1
let feed = 300

note {Spiral pocketing from R[start_radius] to R[end_radius]}

for r = start_radius..end_radius step step {
    G1 X[center_x + r] Y[center_y] F[feed]
}
```

### Algorithm Contributions
- **Describe the algorithm** in your pull request (what problem it solves, how it works).
- **Add comments** in the code for each major step.
- **If porting from another language or paper, cite the source.**
- **Add tests or example GGcode files** in the `GGCODE/` directory.
- **If your algorithm is complex, consider adding a markdown file explaining it in `/docs` or as a comment block.**

### Where to Put Your Scripts
- Place example or reusable scripts in the `GGCODE/` directory.
- Add a short comment at the top with your name/handle and a description.

---

## 📦 Project Structure

See the [README.md](./README.md#project-structure) for an overview of the project layout.

## 📧 Contact

For questions, reach out to [t@doorbase.io](mailto:t@doorbase.io).

---

Thank you for helping make GGcode better! 