
package cppio

/*
#cgo LDFLAGS: -lcppio
#include "../../include/cppio/cppio_c.h"
*/
import "C"

import "unsafe"


var theLineManager unsafe.Pointer

func init() {
	theLineManager = unsafe.Pointer(C.cppio_create_line_manager())
}

type ioLine struct {
	ptr unsafe.Pointer
}

type ioAcceptor struct {
	ptr unsafe.Pointer
}

func CreateClient(endpoint string) ioLine {
	return ioLine { unsafe.Pointer(C.cppio_create_client(theLineManager, C.CString(endpoint))) }
}

func CreateServer(endpoint string) ioAcceptor {
	return ioAcceptor { unsafe.Pointer(C.cppio_create_server(theLineManager, C.CString(endpoint))) }
}

func (line *ioLine) Write(bytes []byte) int {
	return int(C.cppio_line_write(line.ptr, (*C.char)(unsafe.Pointer(&bytes[0])), C.size_t(len(bytes))))
}

func (line *ioLine) Read(bytes []byte) int {
	return int(C.cppio_line_read(line.ptr, (*C.char)(unsafe.Pointer(&bytes[0])), C.size_t(len(bytes))))
}

func (acceptor *ioAcceptor) WaitConnection(milliseconds uint) ioLine {
	return ioLine { unsafe.Pointer(C.cppio_acceptor_wait_connection(acceptor.ptr, C.int(milliseconds)))}
}
