import startEngine from './engine/botle.js';

class EngineApi {
  constructor(instance) {
    this._load_dict = instance.cwrap("load_dict", 'null', ['string']);
    this._compute_next_suggestion = instance.cwrap("compute_next_suggestion", 'string', []);
    this._make_guess = instance.cwrap("make_guess", 'null', ['string']);
    this._cache_key = instance.cwrap("cache_key", 'string', []);
    this._back = instance.cwrap("back", 'boolean', []);
  }

  load_dict(allowed_guesses, possible_secrets, hard_mode) {
    let arg = JSON.stringify({
      'allowed_guesses': allowed_guesses,
      'possible_secrets': possible_secrets,
      'hard_mode': hard_mode,
    });
    try {
      this._load_dict(arg);
    } catch (error) {
      console.error(error);
      throw error;
    }
  }

  compute_multiple_suggest_words(max_words, max_depth) {
    let result = this._compute_multiple_suggest_words(max_words, max_depth);
    return JSON.parse(result);
  }

  compute_next_suggestion() {
    let result = this._compute_next_suggestion();
    let out = JSON.parse(result);
    if ('error' in out || !('ok' in out)) {
      throw out;
    }
    return out['ok'];
  }

  make_guess(guess, pattern) {
    let args = JSON.stringify([guess, pattern]);
    try {
      this._make_guess(args);
    } catch (error) {
      console.log(error);
      throw error;
    }
  }

  cache_key() {
    return this._cache_key();
  }

  back() {
    return this._back();
  }
};

export function createAPI() {
  return startEngine().then((instance) => {
    return new EngineApi(instance);
  });
}
