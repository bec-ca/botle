class Queue {
  constructor() {
    this._in = [];
    this._out = [];
  }

  push(element) {
    this._in.push(element);
  }

  pop() {
    if(this._out.length === 0) {
      while (this._in.length > 0) {
        this._out.push(this._in.pop());
      }
    }
    return this._out.pop();
  }

  length() {
    return this._in.length + this._out.length;
  }

  empty() {
    return this._in.length === 0 && this._out.length === 0;
  }
}

export default class Pipe {
  constructor(send_message, handle_message) {
    this._unacked_messages = {}
    this._unacked_messages_count = 0;
    this._send_message = send_message;
    this._handle_message = handle_message;
    this._seq_num = 0;
    this._message_queue = new Queue();
  }

  send_message(payload) {
    let p = new Promise((resolve) => {
      this._message_queue.push([payload, resolve]);
    });
    this._maybe_send_messages();
    return p;
  }

  _maybe_send_messages() {
    while (!this._message_queue.empty() && this._unacked_messages_count < 5) {
      let [payload, resolve] = this._message_queue.pop();
      this._seq_num++;
      this._unacked_messages[this._seq_num] = true;
      this._unacked_messages_count++;
      this._send_message({
        action: 'payload',
        payload: payload,
        seq_sum: this._seq_num,
      });
      resolve();
    }
  }

  handle_message(msg) {
    let action = msg.action;
    if (action === 'payload') {
      let seq_sum = msg.seq_sum;
      this._send_message({
        action: 'ack',
        seq_num: seq_sum,
      });
      this._handle_message(msg.payload)
    } else if (action === 'ack') {
      let seq_num = msg.seq_num;
      if (!(seq_num in this._unacked_messages)) {
        console.error('Unexpected seq num', seq_num);
        return;
      }
      delete this._unacked_messages[seq_num];
      this._unacked_messages_count--;
      this._maybe_send_messages();
    }
  }
}
