
package bindings_test

import ("testing"
	"reflect"
	"sync"
	"cppio")

func simpleTest(t *testing.T, endpoint string) {
	server_ready := make(chan int)
	var wg sync.WaitGroup
	wg.Add(2)
	data := ([]byte)("foobar-foobar-foobar-0123456789")
	server := func () {
		defer wg.Done()
		s, err := cppio.CreateServer(endpoint)
		if err != nil {
			t.Fail()
			return
		}
		defer s.Close()
		server_ready <- 1
		c := s.WaitConnection(1000)
		defer c.Close()
		buffer := make([]byte, 256, 256)
		rd, err := c.Read(buffer)
		if err != nil {
			t.Fail()
			return
		}
		if rd != len(data) {
			t.Fail()
			return
		}

		if !reflect.DeepEqual(buffer[:rd], data) {
			t.Fail()
			return
		}
	}

	client := func () {
		defer wg.Done()
		<-server_ready
		c, err := cppio.CreateClient(endpoint)
		if err != nil {
			t.Fail()
			return
		}
		defer c.Close()
		_, err = c.Write(data)
		if err != nil {
			t.Fail()
			return
		}
	}

	go server()
	go client()
	wg.Wait()
}

func TestSimpleTcpIo(t *testing.T) {
	simpleTest(t, "tcp://127.0.0.1:5510")
	simpleTest(t, "tcp://127.0.0.1:5511")
}

func TestSimpleInprocIo(t *testing.T) {
	simpleTest(t, "inproc://foobar")
	simpleTest(t, "inproc://foobar2")
}

func TestSimpleLocalIo(t *testing.T) {
	simpleTest(t, "local:///tmp/foobar")
	simpleTest(t, "local:///tmp/foobar2")
}

func TestMessage(t *testing.T) {
	m := cppio.CreateMessage()
	data := ([]byte)("foobar-foobar-foobar-0123456789")
	m.AddFrame(data)

	if m.Size() != 1 {
		t.FailNow()
	}
	if !reflect.DeepEqual(m.GetFrame(0), data) {
		t.FailNow()
	}
}

func messageProtocolTest(t *testing.T, endpoint string) {
	server_ready := make(chan int)
	var wg sync.WaitGroup
	wg.Add(2)
	data := ([]byte)("foobar-foobar-foobar-0123456789")
	server := func () {
		defer wg.Done()
		s, err := cppio.CreateServer(endpoint)
		if err != nil {
			t.Fail()
			return
		}
		defer s.Close()
		server_ready <- 1
		c := s.WaitConnection(1000)
		defer c.Close()
		proto := cppio.CreateMessageProtocol(c)
		m := cppio.CreateMessage()
		err = proto.Read(m)
		if err != nil {
			t.Fail()
			return
		}

		if m.Size() != 1 {
			t.Fail()
			return
		}

		if !reflect.DeepEqual(m.GetFrame(0), data) {
			t.Fail()
			return
		}
	}

	client := func () {
		defer wg.Done()
		<-server_ready
		c, err := cppio.CreateClient(endpoint)
		if err != nil {
			t.Fail()
			return
		}
		defer c.Close()

		proto := cppio.CreateMessageProtocol(c)
		m := cppio.CreateMessage()
		m.AddFrame(data)
		err = proto.Send(m)
		if err != nil {
			t.Fail()
			return
		}
	}

	go server()
	go client()
	wg.Wait()
}

func TestMessageProtocol(t *testing.T) {
}

