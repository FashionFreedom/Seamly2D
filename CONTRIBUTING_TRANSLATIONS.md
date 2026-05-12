# How to Add a New Translation to Seamly2D

This guide outlines the steps required to add a new language translation to Seamly2D, using the automated workflow and scripts provided in the repository.

---

## 1. Use the Automated Workflow (Recommended)

Seamly2D provides a GitHub Actions workflow to automate the creation, registration, and initial population of new translation files.

**To add a new translation:**

1. Go to the GitHub repository's **Actions** tab.
2. Select the workflow named **"Create and Auto-Translate New Language"**.
3. Click **"Run workflow"** and enter your target language code (e.g., `pl_PL` for Polish).
4. The workflow will:
   - Create new translation files (`measurements_LANGCODE.ts` and `seamly2d_LANGCODE.ts`)
   - Register the new language in all required project files
   - Auto-translate the new files using Google Translate
   - Build the project to verify integration
   - Create a pull request with all changes

After the PR is merged, the new language will be available in Seamly2D.

---

## 2. Manual Script Usage (Advanced)

If you prefer to run the script locally or need to debug, you can use the `create_new_ts.py` script directly:

**Usage:**

```sh
python .github/scripts/create_new_ts.py <lang_code>
```

Where `<lang_code>` is your target language code (e.g., `pl_PL`).

This script will:
- Copy template translation files for your language
- Register the new language in `measurements.pro`, `translations.pro`, `translations.pri`, and `def.cpp`

You can then use the `auto_translate_ts.py` script to auto-populate the new files:

```sh
python .github/scripts/auto_translate_ts.py <lang> share/translations/measurements_<lang_code>.ts share/translations/seamly2d_<lang_code>.ts
```

Where `<lang>` is the two-letter language code (e.g., `pl` for Polish).

---

## 2. Update Project Translation Lists

- Add your new language code (e.g., `pl_PL`) to the language lists in the following project files:
  - `share/translations/measurements.pro`
    - Add `pl_PL` to the `LANGUAGES +=` section.
  - `share/translations/translations.pro`
    - Add `pl_PL` to the `LANGUAGES +=` section.
  - `src/app/translations.pri`
    - Add `pl_PL` to the `LOCALES` or `LANGUAGES` list.

## 3. Register the New Locale in Code

- Update the list of supported locales in the codebase:
  - In `src/libs/vmisc/def.cpp`, add your language code to the `SupportedLocales()` function:
    ```cpp
    QStringList SupportedLocales() {
        // ...existing QStringLiterals of locals "en_GB", etc...
        << QStringLiteral("pl_PL");
        // ...existing code...
    }
    ```

## 4. Build and Test

- Rebuild the project to ensure the new translation is included.
- Test the application to verify that the new language appears as an option and that translations are loaded correctly.

---


---

## 5. Updating and Fixing Translations in Weblate

To allow users to fix problems with translations using Weblate:

1. **Push New or Updated Translation Files**
  Ensure all new or updated `.ts` translation files are committed and pushed to the repository branch tracked by Weblate.

2. **Sync Weblate with the Repository**
  Weblate will automatically pull changes from your repository at regular intervals. To make changes appear immediately, trigger a manual sync in the Weblate admin interface (if you have access), or wait for the next scheduled sync.

3. **Check Component Configuration**
  Make sure the new translation files are included in the Weblate component configuration. Weblate should automatically detect new `.ts` files if set up to track all translation files in the directory.

4. **Announce to Contributors**
  Let your community know that the new translation is available for editing on Weblate. Share the Weblate project link and encourage users to review and improve translations.

5. **Review and Merge Contributions**
  As users make suggestions or improvements in Weblate, review and merge these changes back into your repository. Weblate can be configured to automatically push translation updates, or you can manually merge them.

---

For further questions, please refer to the main `CONTRIBUTING.md` or open an issue on GitHub.
