import Pipe from "./pipe";
import {
  createAPI
} from "./api.js";

class State {
  constructor(api) {
    this.api = api;
    this.is_thinking = false;
    this.thinking_id = null;
    this.pipe = new Pipe((msg) => postMessage(msg), (message) => this._handle_message(message));
  }

  async _handle_message(msg) {
    let action = msg['action'];
    let id = msg['id']

    if (action === 'start-thinking') {
      this.thinking_id = id;
      this.is_thinking = true;
      this.schedule_think(id);
    } else if (action === 'stop-thinking') {
      if (this.is_thinking) {
        this.is_thinking = false;
        await this.send_message({
          'action': 'stopped-thinking',
          'update': null,
          'id': id,
        });
      }
    } else if (action === 'load-dict') {
      this.api.load_dict(msg['allowed_guesses'], msg['possible_secrets'], msg['hard_mode']);
      this.is_thinking = false;
    } else if (action === 'make-guess') {
      let guess = msg['word'];
      let match = msg['pattern'];
      this.api.make_guess(guess, match);
      this.is_thinking = false;
    } else if (action === 'cache-key') {
      let key = this.api.cache_key();
      this.send_message({
        'action': 'cache-key',
        'key': key,
        'id': id,
      });
    } else if (action === 'back') {
      let did_go_back = this.api.back()
      this.is_thinking = false;
      this.send_message({
        'action': 'back',
        'did-go-back': did_go_back,
        'id': id,
      });
    } else {
      console.log("Got unexpected action", action);
    }
  }

  send_message(msg) {
    return this.pipe.send_message(msg);
  }

  async think(id) {
    if (!this.is_thinking || id !== this.thinking_id) {
      return;
    }

    var output;
    try {
      output = this.api.compute_next_suggestion();
    } catch (error) {
      console.error(error);
      return;
    }
    if (!output) {
      this.is_thinking = false;
      this.send_message({
        'action': 'stopped-thinking',
        'update': null,
        'id': this.thinking_id,
      });
    } else {
      await this.send_message({
        'action': 'update-think',
        'update': output,
        'id': this.thinking_id,
      });
      this.schedule_think(id);
    }
  }

  handle_message(message) {
    this.pipe.handle_message(message);
  }

  schedule_think(id) {
    setTimeout(() => {
      this.think(id);
    }, 1);
  }

}

async function startup() {

  let api = await createAPI();
  console.log("engine api loaded");

  return new State(api);
}

let state_promise = startup();


onmessage = async function(e) {
  let state = await state_promise;
  state.handle_message(e.data);
}
