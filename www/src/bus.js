export default class Bus {
  contructor() {
    this.handlers = [];
  }


  send(el) {
    for(const h of this.handlers) {
      h(el);
    }
  }

  set_handler(handler) {
    this.handlers = [handler];
  }
};
