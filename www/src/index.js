import React from 'react';
import ReactDOM from 'react-dom';
import './index.css';
import App from './App';
import {
  WorkerAPI
} from './worker_api';

let engine = new WorkerAPI(new Worker(new URL('./web_worker.js',
  import.meta.url)));

ReactDOM.render(
  <React.StrictMode>
    <App engine={engine} />
  </React.StrictMode>,
  document.getElementById('root')
);
