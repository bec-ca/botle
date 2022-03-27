import React from 'react';
import get_text from './translations';
import is_mobile from './device';

import {
  get_allowed_guesses_filename,
  get_possible_secrets_filename
} from "./dicts.js";

async function fetch_lines(url) {
  let response = await fetch(url, {
    cache: "no-cache"
  });
  let text = await response.text();
  return text.split(/\r?\n/).filter(word => word.length === 5);
}

var global_cache = {};

async function fetch_cache(key) {
  if (key in global_cache) {
    console.log('Cache already cached');
    return global_cache[key];
  }
  let url = './cache/' + key + '.json';
  let response = await fetch(url, {
    cache: "no-cache"
  });
  if (!response.ok) {
    return null;
  }
  var out = null;
  try {
    out = await response.json();
  } catch (error) {
    // failed to parse as json, so it is probably not found
    out = null;
  }
  global_cache[key] = out;
  return out;
}

export default class Thinking extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      allowed_guesses: null,
      possible_secrets: null,
      best_words: [],
      word_infos: {},
      dict_loaded: false,
      thinking_word: null,
      thinking_done: false,
      last_best: null,
    };
    props.on_select_bus.set_handler((el) => this.handle_select(el));
    props.on_back_bus.set_handler(() => this.handle_back());

  }

  engine() {
    return this.props.engine;
  }

  handle_select(selection) {
    let word = selection[0];
    let pattern = selection[1];
    this.engine().make_guess(word, pattern);
    this.schedule_think();
  }

  async handle_back() {
    let did_go_back = await this.engine().back();
    if (did_go_back) {
      this.schedule_think();
    } else {
      this.props.on_quit();
    }
  }

  handle_engine_update(update) {
    this.setState((state, props) => {
      let new_word_infos = {
        ...state.word_infos,
      };
      let word = update['guess'];
      if (!(word in new_word_infos) || new_word_infos[word]['avg_guesses'] > update['avg_guesses']) {
        new_word_infos[word] = update;
      } else {
        new_word_infos[word]['max_depth'] = update['max_depth']
      }

      let new_best_words = Object.values(new_word_infos);
      new_best_words.sort((e1, e2) => {
        if (e1['avg_guesses'] !== e2['avg_guesses']) {
          return e1['avg_guesses'] - e2['avg_guesses'];
        } else if (e1['worst_num_guesses'] !== e2['worst_num_guesses']) {
          return e1['worst_num_guesses'] - e2['worst_num_guesses'];
        } else if (e1['guess'] !== e2['guess']) {
          return e1['guess'] < e2['guess'] ? -1 : 1
        }
        return 0
      });
      if (new_best_words.length > 50) {
        new_best_words = new_best_words.slice(0, 50);
      }
      let new_best = new_best_words[0];
      if (this.state.last_best !== new_best) {
        props.on_selection_change(new_best);
      }
      return {
        ...state,
        best_words: new_best_words,
        thinking_word: word,
        word_infos: new_word_infos,
        last_best: new_best,
      };

    });
  }

  thinking_done() {
    this.setState((state) => {
      return {
        ...state,
        thinking_done: true,
      };
    });
  }

  async schedule_think() {
    let key = await this.engine().cache_key();
    let cache = await fetch_cache(key);
    if (cache) {
      console.log("Cache found");
      this.setState((state, props) => {
        props.on_selection_change(cache[0]);
        return {
          ...state,
          best_words: cache,
          word_infos: {},
          last_best: cache[0],
          thinking_word: null,
          thinking_done: true,
        };
      });

    } else {
      this.setState((state) => {
        return {
          ...state,
          best_words: [],
          word_infos: {},
          last_best: null,
          thinking_word: null,
          thinking_done: false,
        };
      });
      console.log("No cache found");
      this.props.engine.start_thinking(
        (update) => this.handle_engine_update(update),
        () => this.thinking_done());
    }
  }

  async prepare_engine() {
    if (!this.state.dict_loaded) {
      let dict = this.props.dict;
      let allowed_guesses_filename = get_allowed_guesses_filename(dict);
      let possible_secrets_filename = get_possible_secrets_filename(dict);
      let allowed_guesses = await fetch_lines(allowed_guesses_filename);
      let possible_secrets = [];
      if (this.props.use_secret_words && possible_secrets_filename) {
        possible_secrets = await fetch_lines(possible_secrets_filename);
      }

      this.props.engine.load_dict(allowed_guesses, possible_secrets, this.props.hard_mode);
      this.schedule_think();

      this.setState((state) => {
        return {
          ...state,
          allowed_guesses: allowed_guesses,
          possible_secrets: possible_secrets,
          dict_loaded: true,
        }
      });
    }
  }

  componentDidMount() {
    this.prepare_engine();
  }

  render_words(best_words) {
    return best_words.map((word) => {
      return (
        <tr key={word['guess']} onClick={() => this.props.on_selection_change(word)}>
          <td className="guess-cell">{word['guess'].toLowerCase()}</td>
          <td className="avg-guess-cell">{word['avg_guesses'].toFixed(4)}</td>
          <td className="worst-guesses-cell">{word['worst_num_guesses']}</td>
          <td className="search-depth-cell">{word['max_depth']}</td>
        </tr>
      );
    });
  }

  render_header() {
    if (this.state.thinking_done) {
      return <div className="search-depth">{get_text("thinking done")}</div>
    } else if (!this.state.thinking_word) {
      return <div className="search-depth">{get_text("initializing")}</div>;
    } else {
      return <div className="search-depth">{get_text("thinking")}</div>;
    }
  }

  render() {
    let className = is_mobile() ? "thinking-container-mobile" : "thinking-container";
    return (
      <div className={className}>
        {this.render_header()}
        <table>
          <thead>
            <tr>
              <th className='guess-cell'>{get_text('word')}</th>
              <th className='avg-guess-cell'>{get_text('avg')}</th>
              <th className='worst-guesses-cell'>{get_text('worst')}</th>
              <th className='search-depth-cell'>{get_text('depth')}</th>
            </tr>
          </thead>
          <tbody>
            {this.render_words(this.state.best_words)}
          </tbody>
        </table>
      </div>
    );
  }

}
