import React from 'react';
import './App.css';
import Thinking from './thinking';
import Bus from './bus';
import get_text from './translations';
import is_mobile from './device';

import {
  createAPI
} from './api';

function get_class(match) {
  if (match === 'X') {
    return 'incorrect';
  } else if (match === '?') {
    return 'correct-letter';
  } else if (match === '!') {
    return 'correct-position';
  } else {
    throw new Error("Bug");
  }
}

function Pattern(props) {
  return [...props.pattern].map((match, idx) => {

    let cell_class_name = is_mobile() ? "cell-mobile" : "cell";
    let class_name = get_class(match);
    let letter = props.word.charAt(idx);
    return <div className={cell_class_name + " " + class_name} key={idx} > {letter} </div>;
  });
}

function PatternBox(props) {
  let class_name = is_mobile() ? "pattern-box-mobile" : "pattern-box";
  return (<div className={class_name} onClick={props.onClick}>
    <Pattern pattern={props.pattern} word={props.word}/>
  </div>);
}


class Playing extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      stack: [],
      selection: null,
      on_select_bus: new Bus(),
      on_back_bus: new Bus(),
    };
  }


  componentDidMount() {}

  handle_click(pattern) {
    this.state.on_select_bus.send([this.state.selection['guess'], pattern]);
    this.setState((state) => {
      return {
        ...state,
        selection: null,
      };
    });
  }

  handle_back() {
    this.state.on_back_bus.send();
    this.setState((state) => {
      return {
        ...state,
        selection: null,
      };
    });
  }

  show_back_button() {
    let class_name = is_mobile() ? "red-button back-button-mobile" : "red-button back-button";
    return (
      <div
      className={class_name}
      onClick={() => this.handle_back()}>
      {get_text('back')}
    </div>);
  }


  on_selection_change(selection) {
    this.setState((state) => {
      return {
        ...state,
        selection: selection,
      };
    });
  }



  render_body() {
    let word = this.state.selection ? this.state.selection['guess'] : get_text('loading');
    let patterns = this.state.selection ? this.state.selection['all_matches'] : [];
    return (
      <div className={is_mobile() ? "body-mobile-container" : "left-container"}>
        <div className={is_mobile() ? 'top-mobile' : 'top'}>
          {this.show_back_button()}
          <div className="recommendation-box">
            <div className='recommendation'>{get_text('selected-guess')}: {word}</div>
          </div>
        </div>
        <div className="instructions">{get_text('instructions')}</div>
        <div className="all-patterns">
          { patterns.map((pattern, idx) =>
            <PatternBox
            pattern={pattern}
            word={word}
            key={idx}
            onClick={() => this.handle_click(pattern)} /> ) }
        </div>
      </div>
    );
  }

  render() {
    let container_class_name = is_mobile() ? "playing-container-mobile" : "playing-container";
    let thinking_container_class_name =
      is_mobile() ? "thinking-container-outer-mobile" : "thinking-container-outer";
    return (
      <div className={container_class_name}>
        {this.render_body()}
        <div className={thinking_container_class_name}>
          <Thinking
            dict={this.props.dict}
            engine={this.props.engine}
            use_secret_words={this.props.use_secret_words}
            hard_mode={this.props.hard_mode}
            on_selection_change={(info) => this.on_selection_change(info)}
            on_select_bus={this.state.on_select_bus}
            on_back_bus={this.state.on_back_bus}
            on_quit={() => this.props.on_quit()} />
        </div>
      </div>
    );
  }
}


class Selector extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      started: false,
      selected_dict: 'wordle',
      hard_mode: false,
      dont_use_secrets: false,
    };

  }

  set_started(started) {
    this.setState((state) => {
      return {
        ...state,
        started: started,
      };
    });
  }

  handle_selection(event) {
    this.setState((state) => {
      return {
        ...state,
        selected_dict: event.target.value,
      };
    });
  }

  handle_select_hard_mode() {
    this.setState((state) => {
      return {
        ...state,
        hard_mode: !state.hard_mode,
      };
    });
  }

  handle_dont_use_secrets() {
    this.setState((state) => {
      return {
        ...state,
        dont_use_secrets: !state.dont_use_secrets,
      };
    });

  }

  render() {
    if (this.state.started) {
      return <Playing engine={this.props.engine} dict={this.state.selected_dict} use_secret_words={!this.state.dont_use_secrets} hard_mode={this.state.hard_mode} on_quit={() => this.set_started(false)} />;
    } else {
      return (
        <div className="selector">
          <label className="dropdown-label">
            {get_text('choose-words')}
            <select value={this.state.selected_dict} className="dict-select" onChange={(event) => this.handle_selection(event)}>
              <option value="wordle">Wordle</option>
              <option value="termoo">Term.ooo</option>
              <option value="letreco">Letreco</option>
              <option value="palavra-do-dia">Palavra do dia</option>
              <option value="super-senha">Super senha</option>
              <option value="xingo">Xingo.site</option>
              <option value="wiki-2k">Wikipedia 2k</option>
              <option value="wiki-4k">Wikipedia 4k</option>
              <option value="wiki-10k">Wikipedia 10k</option>
            </select>
          </label>
          <label className="checkbox-label">
            <input type="checkbox" checked={this.state.hard_mode} onChange={() => {this.handle_select_hard_mode()}} />
            {get_text("hard-mode")}
          </label>
          <label className="checkbox-label">
            <input type="checkbox" checked={this.state.dont_use_secrets} onChange={() => {this.handle_dont_use_secrets()}} />
            {get_text('dont-use-secrets')}
          </label>
          <div className="red-button" onClick={() => this.set_started(true)}>{get_text('start')}</div>
        </div>
      );
    }
  }
}

class App extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      api: null,
    };
  }

  componentDidMount() {
    let future_api = createAPI();
    future_api.then((api) => {
      this.setState((state) => {
        return {
          ...state,
          api: api,
        };
      });
    });
  }

  app_class_name() {
    if (is_mobile()) {
      return "app-mobile";
    } else {
      return "app-desktop";
    }
  }

  render() {
    return (
      <div className="wrapper">
        <div className={"app " + this.app_class_name()}>
          <Selector api={this.state.api} engine={this.props.engine} />
        </div>
      </div>
    );
  }
}


export default App;
