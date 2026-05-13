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

FLAGS_QRC = os.path.join(os.path.dirname(__file__), '../src/libs/vmisc/share/resources/flags.qrc')
FLAGS_DIR = os.path.join(os.path.dirname(__file__), '../src/libs/vmisc/share/resources/flags')

COUNTRY_MAP = {
    'US': 'United States', 'GB': 'United Kingdom', 'BR': 'Brazil', 'FR': 'France', 'DE': 'Germany',
    'IT': 'Italy', 'ES': 'Spain', 'RU': 'Russia', 'UA': 'Ukraine', 'PL': 'Poland', 'FI': 'Finland',
    'NL': 'Netherlands', 'ID': 'Indonesia', 'IL': 'Israel', 'TR': 'Turkey', 'RO': 'Romania',
    'EL': 'Greece', 'CN': 'China', 'CA': 'Canada', 'IN': 'India', 'CZ': 'CzechRepublic',
    'HE': 'Israel', 'PT': 'Brazil', 'HU': 'Hungary', 'SE': 'Sweden', 'SK': 'Slovakia',
    'HR': 'Croatia', 'RS': 'Serbia', 'CO': 'Colombia', 'DO': 'Dominican-Republic',
    'BS': 'Bosnia And Herzegowina',
}

WIKI_FLAG_API = "https://commons.wikimedia.org/w/api.php"


def get_country_from_lang_code(lang_code):
    """Extract country from lang_code (e.g., pt_BR -> Brazil)."""
    parts = lang_code.split('_')
    if len(parts) == 2:
        country = COUNTRY_MAP.get(parts[1].upper(), parts[1].capitalize())
        return country
    return None

def flag_exists(country):
    """Check if flag PNG exists in both qrc and folder."""
    png_name = f"{country}.png"
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

def search_flag_on_commons(country):
    """Search Wikimedia Commons for a PNG flag for the country."""
    params = {
        'action': 'query',
        'format': 'json',
        'prop': 'imageinfo',
        'iiprop': 'url',
        'generator': 'search',
        'gsrsearch': f"Flag of {country} filetype:png", # prioritize PNG
        'gsrlimit': 1
    }
    r = requests.get(WIKI_FLAG_API, params=params)
    data = r.json()
    pages = data.get('query', {}).get('pages', {})
    for page in pages.values():
        info = page.get('imageinfo', [{}])[0]
        url = info.get('url')
        if url and url.lower().endswith('.png'):
            return url
    return None

def download_and_resize_flag(url, country):
    """Download PNG, resize to 24x24, 144dpi, save to flags dir."""
    r = requests.get(url)
    img = Image.open(BytesIO(r.content)).convert('RGBA')
    img = img.resize((24, 24), Image.LANCZOS)
    img.save(os.path.join(FLAGS_DIR, f"{country}.png"), dpi=(144, 144))
    print(f"Saved {country}.png")

def add_flag_to_qrc(country):
    """Add PNG to flags.qrc if not present."""
    tree = ET.parse(FLAGS_QRC)
    root = tree.getroot()
    qresource = root.find('qresource')
    png_name = f"flags/{country}.png"
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
    country = get_country_from_lang_code(lang_code)
    if not country:
        print(f"Could not determine country for lang_code: {lang_code}")
        sys.exit(1)
    if flag_exists(country):
        print(f"Flag for {country} already exists.")
        return
    url = search_flag_on_commons(country)
    if not url:
        print(f"Could not find flag for {country} on Wikimedia Commons.")
        sys.exit(1)
    download_and_resize_flag(url, country)
    add_flag_to_qrc(country)

if __name__ == "__main__":
    main()
