# Contributing to nRF9160 and nRF9151 Feather Examples and Drivers

Thank you for your interest in contributing! This document outlines the guidelines for contributing to this project.

## Developer Certificate of Origin (DCO)

All contributions to this project must be accompanied by a Developer Certificate of Origin sign-off. This certifies that you have the right to submit the contribution under the project's open source license.

By adding a "Signed-off-by" line to your commit messages, you certify that you agree to the [DCO](DCO.txt).

### How to Sign Off Commits

Use the `-s` flag when committing:

```bash
git commit -s -m "Your commit message"
```

This will automatically add a sign-off line to your commit message:

```
Signed-off-by: Your Full Name <your.email@example.com>
```

### Configure Git with Your Real Name and Email

Ensure your Git configuration contains your real name (not a pseudonym) and email address:

```bash
git config user.name "Your Full Name"
git config user.email "your.email@example.com"
```

**Important:** The email address must match the one used in your commits. Pull requests containing commits without proper sign-off will be rejected by our automated checks.

## Contribution Workflow

1. **Fork the repository** and clone it locally
2. **Create a branch** for your changes
3. **Make your changes** following the coding standards below
4. **Test your changes** - ensure builds succeed for affected samples
5. **Commit with sign-off** using `git commit -s`
6. **Push to your fork** and create a pull request

## Coding Standards

- Follow the existing code style and formatting
- Use descriptive variable and function names
- Add comments for complex logic
- Update documentation if you change functionality
- Test your changes on actual hardware when possible

## Sample Guidelines

When adding or modifying samples:

- Include a README.md explaining what the sample does and how to use it
- Add the sample to the build matrix in `.github/workflows/build.yml`
- Ensure the sample builds for both nRF9160 and nRF9151 targets (or document why it's excluded)
- Keep samples focused and minimal - demonstrate one concept well

## Pull Request Process

1. Ensure your PR description clearly describes the problem and solution
2. Reference any related issues
3. Verify that all CI checks pass, including the DCO check
4. Be responsive to feedback and requested changes
5. Once approved, a maintainer will merge your PR

## Questions or Issues?

If you have questions or encounter issues, please open a GitHub issue or discussion.

## License

By contributing, you agree that your contributions will be licensed under the same Apache 2.0 License that covers the project.
