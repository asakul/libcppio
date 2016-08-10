
package bindings_test

import ("testing"
	"reflect"
	"sync"
	"cppio")

func simpleTest(t *testing.T, endpoint string) {
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
		c := s.WaitConnection(1000)
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
}

