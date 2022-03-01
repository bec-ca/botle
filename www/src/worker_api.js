import Pipe from "./pipe";

export class WorkerAPI {
  constructor(worker) {
    this._worker = worker;
    this._on_update = (_) => null;
    this._on_done = () => null;
    this._worker.onmessage = (e) => {
      this._pipe.handle_message(e.data);
    };
    this._message_id = 0;
    this._thinking_id = null;
    this._is_thinking = false;

    this._promises = {}

    this._pipe = new Pipe(
      (msg) => this._worker.postMessage(msg),
      (msg) => this._handle_worker_message(msg));
  }

  send_message(msg) {
    return this._pipe.send_message(msg);
  }

  _get_id() {
    this._message_id++;
    return this._message_id;
  }

  load_dict(allowed_guesses, possible_secrets, hard_mode) {
    let id = this._get_id();
    return this.send_message({
      "action": "load-dict",
      "allowed_guesses": allowed_guesses,
      "possible_secrets": possible_secrets,
      "hard_mode": hard_mode,
      "id": id,
    });
  }

  start_thinking(on_update, on_done) {
    if (this._is_thinking) Promise.resolve();
    this._on_update = on_update;
    this._on_done = on_done;
    let id = this._get_id();
    this._thinking_id = id;
    this._is_thinking = true;
    return this.send_message({
      "action": "start-thinking",
      "id": id,
    });
  }

  async back() {
    let id = this._get_id();
    this._thinking_id = id;
    this._is_thinking = false;
    await this.send_message({
      "action": "back",
      "id": id,
    });
    return new Promise((resolve) => {
      this._promises[id] = (msg) => {
        resolve(msg['did-go-back']);
      };
    });
  }

  make_guess(word, pattern) {
    let id = this._get_id();
    this._is_thinking = false;
    return this.send_message({
      "action": "make-guess",
      'word': word,
      'pattern': pattern,
      "id": id,
    });
  }

  async cache_key() {
    let id = this._get_id();
    await this.send_message({
      "action": "cache-key",
      "id": id,
    });
    return new Promise((resolve) => {
      this._promises[id] = (msg) => {
        resolve(msg['key']);
      };
    });
  }

  _handle_worker_message(msg) {
    let action = msg['action'];
    let id = msg['id'];
    if (id in this._promises) {
      this._promises[id](msg);
      delete this._promises[id];
    } else if (action === 'update-think') {
      if (id === this._thinking_id && this._is_thinking) {
        let update = msg['update'];
        this._on_update(update);
      }
    } else if (action === 'stopped-thinking' && msg['id'] === this._thinking_id) {
      this._is_thinking = false;
      this._on_done();
    } else {
      console.error("unexpected msg", msg);
    }
  }
};
