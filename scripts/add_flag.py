#!/usr/bin/env python3
"""
add_flag.py

Checks if a flag PNG for a given lang_code exists in flags.qrc and the flags folder. If not, downloads a 24x24 PNG flag from Wikimedia, saves it, and updates flags.qrc.

Usage: python add_flag.py <lang_code>
"""

import sys
import os
import requests
import xml.etree.ElementTree as ET
from PIL import Image
from io import BytesIO
try:
    import pycountry
except ImportError:
    print("pycountry is required. Install with: pip install pycountry")
    sys.exit(1)

FLAGS_QRC = os.path.join(os.path.dirname(__file__), '../src/libs/vmisc/share/resources/flags.qrc')
FLAGS_DIR = os.path.join(os.path.dirname(__file__), '../src/libs/vmisc/share/resources/flags')

def get_country_map():
    """Return a dict mapping ISO 3166-1 alpha-2 codes to country names, with XK for Kosovo."""
    country_map = {country.alpha_2: country.name for country in pycountry.countries}
    country_map['XK'] = 'Kosovo'  # XK is not officially in ISO, but used for Kosovo
    return country_map


def get_country_from_lang_code(lang_code):
    """Extract country code and name from lang_code (e.g., pt_BR -> (BR, Brazil))."""
    parts = lang_code.split('_')
    if len(parts) == 2:
        code = parts[1].upper()
        country_map = get_country_map()
        country = country_map.get(code)
        return code, country
    return None, None

def flag_exists(code):
    """Check if flag PNG exists in both qrc and folder."""
    png_name = f"{code}.png"
    # Check file
    if not os.path.exists(os.path.join(FLAGS_DIR, png_name)):
        return False
    # Check qrc
    tree = ET.parse(FLAGS_QRC)
    root = tree.getroot()
    for file_elem in root.iter('file'):
        if file_elem.text == f"flags/{png_name}":
            return True
    return False

def download_flag_from_flagpedia(code):
    """Download PNG flag from flagpedia.net (flagcdn.com) for the given country code."""
    url = f"https://flagcdn.com/24x24/{code.lower()}.png"
    r = requests.get(url, timeout=10)
    if r.status_code == 200:
        return r.content
    else:
        print(f"Flag not found for {code} at {url}")
        return None

def save_flag_png(png_bytes, code):
    """Save PNG bytes to flags dir as code.png, ensure 24x24, 144dpi."""
    img = Image.open(BytesIO(png_bytes)).convert('RGBA')
    img = img.resize((24, 24), Image.LANCZOS)
    img.save(os.path.join(FLAGS_DIR, f"{code}.png"), dpi=(144, 144))
    print(f"Saved {code}.png")

def add_flag_to_qrc(code):
    """Add PNG to flags.qrc if not present."""
    tree = ET.parse(FLAGS_QRC)
    root = tree.getroot()
    qresource = root.find('qresource')
    png_name = f"flags/{code}.png"
    for file_elem in qresource.iter('file'):
        if file_elem.text == png_name:
            return
    new_elem = ET.Element('file')
    new_elem.text = png_name
    qresource.append(new_elem)
    tree.write(FLAGS_QRC, encoding='utf-8', xml_declaration=True)
    print(f"Added {png_name} to flags.qrc")

def main():
    if len(sys.argv) < 2:
        print("Usage: python add_flag.py <lang_code>")
        sys.exit(1)
    lang_code = sys.argv[1]
    code, country = get_country_from_lang_code(lang_code)
    if not code or not country:
        print(f"Could not determine country for lang_code: {lang_code}")
        sys.exit(1)
    if flag_exists(code):
        print(f"Flag for {country} ({code}) already exists.")
        return
    png_bytes = download_flag_from_flagpedia(code)
    if not png_bytes:
        print(f"Could not find flag for {country} ({code}) on flagpedia.net.")
        sys.exit(1)
    save_flag_png(png_bytes, code)
    add_flag_to_qrc(code)

if __name__ == "__main__":
    main()
