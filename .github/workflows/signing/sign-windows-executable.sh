#!/bin/bash
# Validate the jar file
echo '290377fc4f593256200b3ea4061b7409e8276255f449d4c6de7833faf0850cc1  jsign.jar' | sha256sum -c
# TODO exit if it doesn't match the hash


java -jar jsign.jar \
    --storetype GOOGLECLOUD \
    --storepass "$(gcloud auth print-access-token)" \
    --keystore "projects/$SEAMLY_SIGNING_PROJECT_ID/locations/$SEAMLY_SIGNING_LOCATION/keyRings/$SEAMLY_SIGNING_KEYRING_NAME" \
    --alias "$SEAMLY_SIGNING_KEY_NAME" \
    --certfile "codesign-chain.pem" \
    --tsmode RFC3161 \
    --tsaurl http://timestamp.globalsign.com/tsa/r6advanced1 \
    seamly2d.exe
