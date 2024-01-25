package client

var globalHostAddr string = "localhost:10502"

func Init(addr string) {
	globalHostAddr = addr
}
