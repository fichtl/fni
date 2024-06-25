package common

import "fmt"

// Ref: https://www.iana.org/assignments/icmp-parameters/icmp-parameters.xhtml
const (
	ICMPv4TypeEchoReply              = 0
	ICMPv4TypeDestinationUnreachable = 3
	ICMPv4TypeSourceQuench           = 4 // deprecated
	ICMPv4TypeRedirect               = 5
	ICMPv4TypeAlternateHostAddress   = 7 // deprecated
	ICMPv4TypeEchoRequest            = 8
	ICMPv4TypeRouterAdvertisement    = 9
	ICMPv4TypeRouterSolicitation     = 10
	ICMPv4TypeTimeExceeded           = 11
	ICMPv4TypeParameterProblem       = 12
	ICMPv4TypeTimestampRequest       = 13
	ICMPv4TypeTimestampReply         = 14
	ICMPv4TypeInfoRequest            = 15 // deprecated
	ICMPv4TypeInfoReply              = 16 // deprecated
	ICMPv4TypeAddressMaskRequest     = 17 // deprecated
	ICMPv4TypeAddressMaskReply       = 18 // deprecated
	// 19 reserved for security
	// 20-29 reserved for robustness experiment
	ICMPv4TypeTraceroute                = 30 // deprecated
	ICMPv4TypeDatagramConversionError   = 31 // deprecated
	ICMPv4TypeMobileHostRedirect        = 32 // deprecated
	ICMPv4TypeIPv6WhereAreYou           = 33 // deprecated
	ICMPv4TypeIPv6IAmHere               = 34 // deprecated
	ICMPv4TypeMobileRegistrationRequest = 35 // deprecated
	ICMPv4TypeMobileRegistrationReply   = 36 // deprecated
	ICMPv4TypeDomainNameRequest         = 37 // deprecated
	ICMPv4TypeDomainNameReply           = 38 // deprecated
	ICMPv4TypeSkip                      = 39 // deprecated
	ICMPv4TypePhoturis                  = 40
	ICMPv4TypeIPv4IdentifyingMessage    = 41 // experimental mobility protocols such as Seamoby
	ICMPv4TypeExtendedEchoRequest       = 42
	ICMPv4TypeExtendedEchoReply         = 43
	ICMPv4TypeExperimental253           = 253
	ICMPv4TypeExperimental254           = 254
)

var ICMPv4Types = map[uint8]string{
	ICMPv4TypeEchoReply:              "EchoReply",
	ICMPv4TypeDestinationUnreachable: "DestinationUnreachable",
	ICMPv4TypeRedirect:               "Redirect",
	ICMPv4TypeEchoRequest:            "EchoRequest",
	ICMPv4TypeRouterAdvertisement:    "RouterAdvertisement",
	ICMPv4TypeRouterSolicitation:     "RouterSolicitation",
	ICMPv4TypeTimeExceeded:           "TimeExceeded",
	ICMPv4TypeParameterProblem:       "ParameterProblem",
	ICMPv4TypeTimestampRequest:       "TimestampRequest",
	ICMPv4TypeTimestampReply:         "TimestampReply",
	ICMPv4TypePhoturis:               "Photuris",
	ICMPv4TypeExtendedEchoRequest:    "ExtendedEchoRequest",
	ICMPv4TypeExtendedEchoReply:      "ExtendedEchoReply",
}

type ICMPv4Type uint8

func (t ICMPv4Type) String() string {
	if v, ok := ICMPv4Types[uint8(t)]; !ok { // not interested
		return fmt.Sprintf("UnsupportedType%d", t)
	} else {
		return v
	}
}
