#!/usr/bin/env bash
# This script starts the translation workflow for a given language code.
# example usage: ./start_translation_workflow.sh --lang-code el_GR
set -euo pipefail

LANG_CODE=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --lang-code)
      LANG_CODE="$2"
      shift 2
      ;;
    *)
      echo "Unknown argument: $1"
      echo "Usage: $0 --lang-code <lang_code>"
      exit 1
      ;;
  esac
done

if [[ -z "$LANG_CODE" ]]; then
  echo "Missing required parameter: --lang-code"
  echo "Usage: $0 --lang-code el_GR"
  exit 1
fi

gh workflow run .github/workflows/create-and-auto-translate.yml --ref test-translate  --field "lang_code=$LANG_CODE"