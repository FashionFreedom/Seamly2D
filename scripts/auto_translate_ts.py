#---------------------------------------------------------------------------------------------------------------------
#  @file   auto_translate_ts.py
#  @author Seamly2D Contributors
#  @date   12 May, 2026
#
#  @brief
#  Script to auto-translate Qt .ts files using Google Translate for Seamly2D.
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
from googletrans import Translator
from xml.etree import ElementTree as ET

def translate_ts_file(ts_path, dest_lang):
    tree = ET.parse(ts_path)
    root = tree.getroot()
    translator = Translator()
    changed = False

    for context in root.findall('context'):
        for message in context.findall('message'):
            source = message.find('source')
            translation = message.find('translation')
            if translation is not None and (translation.text is None or translation.text.strip() == ''):
                translated = translator.translate(source.text, dest=dest_lang).text
                translation.text = translated
                changed = True

    if changed:
        tree.write(ts_path, encoding='utf-8')
        print(f"Translated: {ts_path}")
    else:
        print(f"No changes: {ts_path}")

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: python auto_translate_ts.py <lang_code> <file1.ts> [<file2.ts> ...]")
        sys.exit(1)
    lang_code = sys.argv[1]
    for ts_file in sys.argv[2:]:
        translate_ts_file(ts_file, lang_code)
