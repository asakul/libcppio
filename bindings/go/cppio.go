
package cppio

/*
#cgo LDFLAGS: -lcppio
#include "../../include/cppio/cppio_c.h"
*/
import "C"

import ("unsafe"
	"errors"
	"runtime"
	"fmt")

const (OReceiveTimeout = int(C.cppio_receive_timeout)
	OSendTimeout = int(C.cppio_send_timeout))

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

type IoLine struct {
	ptr unsafe.Pointer
}

type IoAcceptor struct {
	ptr unsafe.Pointer
}

type Message struct {
	ptr unsafe.Pointer
}

type MessageProtocol struct {
	ptr unsafe.Pointer
}

func (line *IoLine) IsValid() bool {
	return line.ptr != nil
}

func CreateClient(endpoint string) (line IoLine, err Error) {
	ptr := unsafe.Pointer(C.cppio_create_client(theLineManager, C.CString(endpoint)))
	if ptr == nil {
		return IoLine { nil }, newError(fmt.Errorf("Unable to connect to: %s", endpoint), 0)
	} else {
		return IoLine { ptr }, nil
	}
}

func (line *IoLine) Close() error {
	C.cppio_destroy_line(line.ptr)
	return nil
}

func CreateServer(endpoint string) (acceptor IoAcceptor, err Error) {
	ptr := unsafe.Pointer(C.cppio_create_server(theLineManager, C.CString(endpoint)))
	if ptr == nil {
		return IoAcceptor { nil }, newError(fmt.Errorf("Unable to create acceptor on: %s", endpoint), 0)
	} else {
		return IoAcceptor { ptr }, nil
	}
}

func (acceptor *IoAcceptor) Close() {
	C.cppio_destroy_acceptor(acceptor.ptr)
}

func (line *IoLine) Write(bytes []byte) (n int, err Error) {
	wr := int(C.cppio_line_write(line.ptr, (*C.char)(unsafe.Pointer(&bytes[0])), C.size_t(len(bytes))))
	if wr < 0 {
		return 0, newError(fmt.Errorf("cppio error: %d", wr), wr)
	} else if wr < len(bytes) {
		return wr, newError(fmt.Errorf("Truncated write: %d"), 0)
	} else {
		return wr, nil
	}
}

func (line *IoLine) Read(bytes []byte) (n int, err Error) {
	rd := int(C.cppio_line_read(line.ptr, (*C.char)(unsafe.Pointer(&bytes[0])), C.size_t(len(bytes))))
	if rd < 0 {
		return 0, newError(fmt.Errorf("cppio error: %d", rd), rd)
	} else {
		return rd, nil
	}
}

func (line *IoLine) setReceiveTimeout(value int) error {
	C.cppio_line_set_option(line.ptr, C.cppio_receive_timeout, unsafe.Pointer(&value))
	return nil
}

func (line *IoLine) setSendTimeout(value int) error {
	C.cppio_line_set_option(line.ptr, C.cppio_send_timeout, unsafe.Pointer(&value))
	return nil
}

func (line *IoLine) SetOptionInt(option int, value int) error {
	switch(option) {
	case OReceiveTimeout:
		return line.setReceiveTimeout(value)
	case OSendTimeout:
		return line.setSendTimeout(value)
	default:
		return errors.New("Unknown option")
	}
}

func (acceptor *IoAcceptor) WaitConnection(milliseconds uint) IoLine {
	return IoLine { unsafe.Pointer(C.cppio_acceptor_wait_connection(acceptor.ptr, C.int(milliseconds)))}
}

func CreateMessage() *Message {
	m := Message { unsafe.Pointer(C.cppio_create_message()) }
	runtime.SetFinalizer(&m, func (msg *Message) {
		C.cppio_destroy_message(msg.ptr)
	})
	return &m
}

func (m *Message) AddFrame(data []byte) {
	C.cppio_message_add(m.ptr, (*C.char)(unsafe.Pointer(&data[0])), C.size_t(len(data)))
}

func (m *Message) Size() uint {
	return uint(C.cppio_message_size(m.ptr))
}

func (m *Message) GetFrame(frameNumber uint) []byte {
	frame := C.cppio_message_get_frame(m.ptr, C.size_t(frameNumber))
	frameLength := uint(C.cppio_message_get_frame_length(m.ptr, C.size_t(frameNumber)))
	b := make([]byte, frameLength)
	for i:= uint(0); i < frameLength; i++ {
		b[i] = *(*byte)(unsafe.Pointer(uintptr(unsafe.Pointer(frame)) + uintptr(i)))
	}
	return b
}

func (m *Message) Clear() {
	C.cppio_message_clear(m.ptr)
}

func CreateMessageProtocol(line IoLine) MessageProtocol {
	return MessageProtocol { unsafe.Pointer(C.cppio_create_messageprotocol(line.ptr)) }
}

func (p *MessageProtocol) Close() {
	C.cppio_destroy_messageprotocol(p.ptr)
}

func (p *MessageProtocol) Read(m *Message) Error {
	retCode := int(C.cppio_messageprotocol_read(p.ptr, m.ptr))
	if retCode <= 0 {
		return newError(fmt.Errorf("Unable to read message: %d", retCode), retCode)
	} else {
		return nil
	}
}

func (p *MessageProtocol) Send(m *Message) Error {
	retCode := int(C.cppio_messageprotocol_send(p.ptr, m.ptr))
	if retCode <= 0 {
		return newError(fmt.Errorf("Unable to send message: %d", retCode), retCode)
	} else {
		return nil
	}
}
