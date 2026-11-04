### RELEASES

StudWorx publishes a release on GitHub, built and published by `.github/workflows/continuous.yml` on every push to `main`.

- The `release` job runs once the `build-deb`, `build-rpm`, `build-appimage`, `build-macos` and `build-windows` jobs pass.
- The version is the release date, read from `SW_VERSION_TEXT` in `source/application/sw_global.h`, moved by hand with `tools/studworx-version --date` and bumped by the same script as the pre-push hook.
- The release notes are that version's notice in `NOTICE.md`.
- One release carries the latest build of `main`: each push replaces the release and its tag at the pushed commit, and earlier date versions are removed with their tags.
- Packages: `.deb` and `.rpm` (x86_64, aarch64), AppImage (x86_64, aarch64), macOS `.dmg` (universal) and Windows `.exe` (x64).
- Completely automated: pushing `main` rebuilds and republishes the release.
