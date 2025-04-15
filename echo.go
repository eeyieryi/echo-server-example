package main

import (
	"errors"
	"fmt"
	"io"
	"net"
	"os"
)

func main() {
	ln, err := net.Listen("tcp", "127.0.0.1:8080")
	if err != nil {
		panic(err)
	}
	defer func() {
		err := ln.Close()
		if err != nil {
			panic(err)
		}
	}()

	for {
		conn, err := ln.Accept()
		if err != nil {
			continue
		}

		go func(conn net.Conn) {
			fmt.Println("Connection accepted!")
			if err != nil {
				fmt.Fprintf(os.Stderr, "ERROR: Could not accept connection: %s\n", err.Error())
			}
			defer func() {
				fmt.Println("Closing connection!")
				if err := conn.Close(); err != nil {
					fmt.Fprintf(os.Stderr, "ERROR: Could not close client connection: %s\n", err.Error())
				}
			}()
			for {
				buf := make([]byte, 4096, 4096)
				var n int
				n, err = conn.Read(buf)
				if err != nil {
					if !errors.Is(io.EOF, err) {
						fmt.Fprintf(os.Stderr, "ERROR: Could not read from client: %s\n", err.Error())
					}
					break
				}
				if n == 0 {
					continue
				}

				writtenN, err := conn.Write(buf[:n])
				if err != nil {
					fmt.Fprintf(os.Stderr, "ERROR: Could not write to the client: %s\n", err.Error())
					break
				}
				if writtenN != n {
					fmt.Fprintf(os.Stderr, "ERROR: read %d but only wrote %d\n", n, writtenN)
					break
				}
			}
		}(conn)
	}
}
