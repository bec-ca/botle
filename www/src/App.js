import React from 'react';
import './App.css';
import Thinking from './thinking';
import Bus from './bus';

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
    let class_name = get_class(match);
    let letter = props.word.charAt(idx);
    return <div className={"cell " + class_name} key={idx} > {letter} </div>;
  });
}

function PatternBox(props) {
  return (<div className="pattern-box" onClick={props.onClick}>
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
    this.setState((state ) => {
      return {
        ...state,
        selection: null,
      };
    });
  }

  show_back_button() {
    return <div className="red-button back-button" onClick={() => this.handle_back()}>Back</div>
  }


  on_selection_change(selection) {
    this.setState((state) => {
      return {
        ...state,
        selection: selection,
      };
    });
  }

  render_left() {
    let word = this.state.selection ? this.state.selection['guess'] : "Loading...";
    let patterns = this.state.selection ? this.state.selection['all_matches'] : [];
    return (
      <div className="left-container">
        <div className='top'>
          {this.show_back_button()}
          <div className='top-info'>
            <div className='recommendation'>Selected guess:</div>
            <div className='selected-word'>{word}</div>
          </div>
        </div>
        <div>Instructions: Type in the suggested word above on wordle and pick the color pattern that you got:</div>
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
    return (
      <div className="playing-container">
        {this.render_left()}
        <Thinking
          dict={this.props.dict} engine={this.props.engine}
          hard_mode={this.props.hard_mode}
          on_selection_change={(info) => this.on_selection_change(info)}
          on_select_bus={this.state.on_select_bus}
          on_back_bus={this.state.on_back_bus}
          on_quit={() => this.props.on_quit()} />
      </div>
    );
  }
}


class Selector extends React.Component {
    constructor(props) {
      super(props);
      this.state = {
        started: false,
        selected_dict: 'wordle-secret',
        hard_mode: false,
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

    render() {
        if (this.state.started) {
          return <Playing engine={this.props.engine} dict={this.state.selected_dict} hard_mode={this.state.hard_mode} on_quit={() => this.set_started(false)} />;
        } else {
          return (
              <div className="selector">
          <label className="dropdown-label">
            Choose the set of words to use:
            <select value={this.state.selected_dict} className="dict-select" onChange={(event) => this.handle_selection(event)}>
              <option value="wordle-secret">Wordle with secrets</option>
              <option value="termoo-secrets">Term.ooo secrets</option>
              <option value="wordle">Wordle</option>
              <option value="termoo">Term.ooo</option>
              <option value="xingo">Xingo.site</option>
              <option value="wiki-2k">Wikipedia 2k</option>
              <option value="wiki-4k">Wikipedia 4k</option>
              <option value="wiki-10k">Wikipedia 10k</option>
            </select>
          </label>
          <label className="checkbox-label">
            <input type="checkbox" checked={this.state.hard_mode} onChange={() => {this.handle_select_hard_mode()}} />
            Hard mode
          </label>
          <div className="red-button" onClick={() => this.set_started(true)}>Start</div>
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

  render() {
    return (
      <div className="wrapper">
        <div className="App">
          <Selector api={this.state.api} engine={this.props.engine} />
        </div>
      </div>
    );
  }
}


export default App;
