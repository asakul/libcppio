
package cppio

/*
#cgo LDFLAGS: -lcppio
#include "../../include/cppio/cppio_c.h"
*/
import "C"

import ("unsafe"
	"fmt")

const (
	eTimeout = -1
	eConnectionLost = -2
	eBufferTooBig = -3
)

type Error interface {
	error
	Timeout() bool
}

type cppioError struct {
	error
	retCode int
}

func (e cppioError) Timeout() bool {
	return e.retCode == eTimeout
}

func newError(e error, retCode int) cppioError {
	return cppioError { e, retCode }
}

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

func CreateClient(endpoint string) (line ioLine, err Error) {
	ptr := unsafe.Pointer(C.cppio_create_client(theLineManager, C.CString(endpoint)))
	if ptr == nil {
		return ioLine { nil }, newError(fmt.Errorf("Unable to connect to: %s", endpoint), 0)
	} else {
		return ioLine { ptr }, nil
	}
}

func (line *ioLine) Close() {
	C.cppio_destroy_line(line.ptr)
}

func CreateServer(endpoint string) (acceptor ioAcceptor, err Error) {
	ptr := unsafe.Pointer(C.cppio_create_server(theLineManager, C.CString(endpoint)))
	if ptr == nil {
		return ioAcceptor { nil }, newError(fmt.Errorf("Unable to create acceptor on: %s", endpoint), 0)
	} else {
		return ioAcceptor { ptr }, nil
	}
}

func (acceptor *ioAcceptor) Close() {
	C.cppio_destroy_acceptor(acceptor.ptr)
}

func (line *ioLine) Write(bytes []byte) (n int, err Error) {
	wr := int(C.cppio_line_write(line.ptr, (*C.char)(unsafe.Pointer(&bytes[0])), C.size_t(len(bytes))))
	if wr < 0 {
		return 0, newError(fmt.Errorf("cppio error: %d", wr), wr)
	} else if wr < len(bytes) {
		return wr, newError(fmt.Errorf("Truncated write: %d"), 0)
	} else {
		return wr, nil
	}
}

func (line *ioLine) Read(bytes []byte) (n int, err Error) {
	rd := int(C.cppio_line_read(line.ptr, (*C.char)(unsafe.Pointer(&bytes[0])), C.size_t(len(bytes))))
	if rd < 0 {
		return 0, newError(fmt.Errorf("cppio error: %d", rd), rd)
	} else {
		return rd, nil
	}
}

func (acceptor *ioAcceptor) WaitConnection(milliseconds uint) ioLine {
	return ioLine { unsafe.Pointer(C.cppio_acceptor_wait_connection(acceptor.ptr, C.int(milliseconds)))}
}
