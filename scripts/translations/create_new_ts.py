
#---------------------------------------------------------------------------------------------------------------------
#  @file   create_new_ts.py
#  @author slspencer
#  @date   12 May, 2026
#
#  @brief
#  Script to create and register new Qt translation (.ts) files for Seamly2D.
#
#  @copyright
#  This source code is part of the Seamly2D project, a pattern making
#  program to create and model patterns of clothing.
#  Copyright (C) 2017-2026 Seamly2D project
#  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
#
#  Seamly2D is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Seamly2D is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
#---------------------------------------------------------------------------------------------------------------------
import sys
import os
import shutil
from xml.etree import ElementTree as ET

TEMPLATE_FILES = [
    'measurements_en_US.ts',
    'seamly2d_en_US.ts',
    'qtbase_en_US.ts'
]

def add_lang_to_pro_file(filepath, lang_code):
    """
    @brief Add a language code to the LANGUAGES list in a .pro file.
    @param filepath Path to the .pro file.
    @param lang_code Language code to add (e.g., 'pl_PL').
    """
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except Exception as e:
        print(f"[ERROR] Could not read {filepath}: {e}")
        return
    new_lines = []
    added = False
    for line in lines:
        # Look for the LANGUAGES += block
        if line.strip().startswith('LANGUAGES +='):
            new_lines.append(line)
            continue
        # Add the new language after tr_TR for consistency
        if line.strip().endswith('tr_TR') and not added:
            new_lines.append(line.rstrip() + ' \\\n    ' + lang_code + '\n')
            added = True
        else:
            new_lines.append(line)
    if not added:
        # Fallback: append at end if not found
        new_lines.append('    ' + lang_code + '\n')
    try:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.writelines(new_lines)
        print(f"Added {lang_code} to {filepath}")
    except Exception as e:
        print(f"[ERROR] Could not write to {filepath}: {e}")

def add_lang_to_translations_pri(filepath, lang_code):
    """
    @brief Add a language code to the LANGUAGES list in a .pri file.
    @param filepath Path to the .pri file.
    @param lang_code Language code to add (e.g., 'pl_PL').
    """
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except Exception as e:
        print(f"[ERROR] Could not read {filepath}: {e}")
        return
    added = False
    for i, line in enumerate(lines):
        # Look for the LANGUAGES += block
        if line.strip().startswith('LANGUAGES +='):
            # Find last language in the block
            j = i
            while j+1 < len(lines) and '\\' in lines[j]:
                j += 1
            # Insert after last LANGUAGES line
            lines[j] = lines[j].rstrip() + ' \\\n    ' + lang_code + '\n'
            added = True
            break
    if not added:
        # Fallback: append at end if not found
        lines.append('    ' + lang_code + '\n')
    try:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.writelines(lines)
        print(f"Added {lang_code} to {filepath}")
    except Exception as e:
        print(f"[ERROR] Could not write to {filepath}: {e}")

def add_locale_to_def_cpp(filepath, lang_code):
    """
    @brief Add a language code to the SupportedLocales() function in def.cpp.
    @param filepath Path to def.cpp.
    @param lang_code Language code to add (e.g., 'pl_PL').
    """
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except Exception as e:
        print(f"[ERROR] Could not read {filepath}: {e}")
        return
    added = False
    for i, line in enumerate(lines):
        # Look for the locales QStringList
        if 'QStringList locales = QStringList()' in line:
            # Find last locale entry
            j = i
            while j+1 < len(lines) and 'QStringLiteral' in lines[j+1]:
                j += 1
            # Insert before return if not already present
            if not any(lang_code in l for l in lines[i:j+1]):
                last_entry = lines[j].rstrip()
                if last_entry.endswith(';'):
                    last_entry = last_entry[:-1]
                lines[j] = last_entry + f"\n                                              << QStringLiteral(\"{lang_code}\");\n"
                added = True
            break
    try:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.writelines(lines)
        print(f"Added {lang_code} to SupportedLocales() in {filepath}")
    except Exception as e:
        print(f"[ERROR] Could not write to {filepath}: {e}")

def create_new_translation_files(lang_code, dest_dir):
    """
    @brief Create new translation files for a given language and update project files.
    @param lang_code Language code to add (e.g., 'pl_PL').
    @param dest_dir Directory containing translation templates and .pro files.
    """
    # Check if any new translation file already exists
    for template in TEMPLATE_FILES:
        src = os.path.join(dest_dir, template)
        new_file = src.replace('en_US', lang_code)
        if os.path.exists(new_file):
            print(f"File already exists: {new_file}. Exiting.")
            sys.exit(1)
        if not os.path.exists(src):
            print(f"[ERROR] Template file not found: {src}")
            continue
        try:
            shutil.copy(src, new_file)
        except Exception as e:
            print(f"[ERROR] Could not copy {src} to {new_file}: {e}")
            continue
        try:
            tree = ET.parse(new_file)
            root = tree.getroot()
            for context in root.findall('context'):
                for message in context.findall('message'):
                    translation = message.find('translation')
                    if translation is not None:
                        translation.text = ''
            tree.write(new_file, encoding='utf-8')
            print(f"Created: {new_file}")
        except Exception as e:
            print(f"[ERROR] Could not process XML for {new_file}: {e}")
            continue
    # Proceed to create new translation files
    for template in TEMPLATE_FILES:
        src = os.path.join(dest_dir, template)
        if not os.path.exists(src):
            print(f"Template file not found: {src}")
            continue
        new_file = src.replace('en_US', lang_code)
        shutil.copy(src, new_file)
        # Clear all <translation> tags in the new file
        tree = ET.parse(new_file)
        root = tree.getroot()
        for context in root.findall('context'):
            for message in context.findall('message'):
                translation = message.find('translation')
                if translation is not None:
                    translation.text = ''  # Clear translation text
        tree.write(new_file, encoding='utf-8')
        print(f"Created: {new_file}")
    # Update project files to register the new language
    add_lang_to_pro_file(os.path.join(dest_dir, 'measurements.pro'), lang_code)
    add_lang_to_pro_file(os.path.join(dest_dir, 'translations.pro'), lang_code)
    add_lang_to_translations_pri(os.path.join('src', 'app', 'translations.pri'), lang_code)
    add_locale_to_def_cpp(os.path.join('src', 'libs', 'vmisc', 'def.cpp'), lang_code)

if __name__ == '__main__':
    """
    @brief Main entry point. Usage: python create_new_ts.py <lang_code>
    """
    print("[DEBUG] Starting create_new_ts.py...")
    print(f"[DEBUG] sys.argv: {sys.argv}")
    if len(sys.argv) < 2:
        print("Usage: python create_new_ts.py <lang_code>")
        sys.exit(1)
    lang_code = sys.argv[1]
    print(f"[DEBUG] lang_code: {lang_code}")
    measurements_pro = os.path.join('share', 'translations', 'measurements.pro')
    print(f"[DEBUG] Checking if {lang_code} exists in {measurements_pro}")
    # Check if lang_code already exists in measurements.pro
    with open(measurements_pro, 'r', encoding='utf-8') as f:
        pro_contents = f.read()
    if lang_code in pro_contents:
        print(f"Language code {lang_code} already exists in measurements.pro. Exiting.")
        sys.exit(0)
    dest_dir = os.path.join('share', 'translations')
    print(f"[DEBUG] dest_dir: {dest_dir}")
    create_new_translation_files(lang_code, dest_dir)
    print("[DEBUG] Finished create_new_ts.py.")
