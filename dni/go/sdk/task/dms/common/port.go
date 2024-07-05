package common

import (
	"strconv"
)

type PORT uint16

// TODO: read from /etc/services
//
// References:
// 1. https://www.rfc-editor.org/rfc/rfc6335.html
// 2. https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.txt
const (
	PORT_CHARGEN   PORT = 19  // Character Generator Protocol
	PORT_FTP       PORT = 21  // File Transfer Protocol
	PORT_SSH       PORT = 22  // Secure Shell
	PORT_DNS       PORT = 53  // Domain Name System
	PORT_TFTP      PORT = 69  // Trivial File Transfer Protocol
	PORT_HTTP      PORT = 80  // Hyper Text Transfer Protocol
	PORT_PORTMAP   PORT = 111 // sunRPC
	PORT_NTP       PORT = 123 // Network Time Protocol
	PORT_SNMP      PORT = 161 // Simple Network Management Protocol
	PORT_LDAP      PORT = 389 // LightWeight Directory Access Protocol
	PORT_HTTPS     PORT = 443 // HTTP over SecureSocket Layer
	PORT_LDAPS     PORT = 636
	PORT_MSSQL     PORT = 1433
	PORT_SSDP      PORT = 1900 // Simple Server Discovery Protocol
	PORT_MEMCACHED PORT = 11211
)

// TODO 1023 maybe not suitble
const MAX_SYSTEM_PORT PORT = 1023

var ServiceMap = map[PORT]string{
	PORT_FTP:       "ftp",
	PORT_SSH:       "ssh",
	PORT_DNS:       "domain",
	PORT_TFTP:      "tftp",
	PORT_HTTP:      "http",
	PORT_PORTMAP:   "portmap",
	PORT_NTP:       "ntp",
	PORT_SNMP:      "snmp",
	PORT_LDAP:      "ldap",
	PORT_HTTPS:     "https",
	PORT_LDAPS:     "ssl-ldap",
	PORT_MSSQL:     "mssql",
	PORT_SSDP:      "ssdp",
	PORT_MEMCACHED: "memcache",
}

func (p PORT) String() string {
	return strconv.FormatInt(int64(p), 10)
}

func IsSystemPort(port PORT) bool {
	return 0 < port && port <= MAX_SYSTEM_PORT
}
