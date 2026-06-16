#!/usr/bin/env sh
# Point a DuckDNS subdomain at this VM's current public IP.
#
# DuckDNS auto-detects the caller's public IP when the ip= parameter is left
# empty, so this works without hardcoding the address. Configuration comes from
# the environment (installed at /etc/duckdns.env by bootstrap.sh):
#   DUCKDNS_SUBDOMAIN  the label only, e.g. "myuni" for myuni.duckdns.org
#   DUCKDNS_TOKEN      the token from the DuckDNS dashboard
#
# Exits non-zero (and logs) if DuckDNS does not answer "OK".

set -eu

: "${DUCKDNS_SUBDOMAIN:?set DUCKDNS_SUBDOMAIN}"
: "${DUCKDNS_TOKEN:?set DUCKDNS_TOKEN}"

response=$(curl -fsS \
	"https://www.duckdns.org/update?domains=${DUCKDNS_SUBDOMAIN}&token=${DUCKDNS_TOKEN}&ip=")

if [ "$response" = "OK" ]; then
	echo "duckdns: ${DUCKDNS_SUBDOMAIN}.duckdns.org updated OK"
else
	echo "duckdns: update FAILED (response: '${response}')" >&2
	exit 1
fi
