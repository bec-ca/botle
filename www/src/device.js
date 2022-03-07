function has_touch_screen() {
  if ("maxTouchPoints" in navigator) {
    return navigator.maxTouchPoints > 0;
  } else if ("msMaxTouchPoints" in navigator) {
    return navigator.msMaxTouchPoints > 0;
  } else {
    var mQ = window.matchMedia && matchMedia("(pointer:coarse)");
    if (mQ && mQ.media === "(pointer:coarse)") {
      return !!mQ.matches;
    } else if ('orientation' in window) {
      return true; // deprecated, but good fallback
    } else {
      // Only as a last resort, fall back to user agent sniffing
      var UA = navigator.userAgent;
      return (
        /\b(BlackBerry|webOS|iPhone|IEMobile)\b/i.test(UA) ||
        /\b(Android|Windows Phone|iPad|iPod)\b/i.test(UA)
      );
    }
  }
}

function check_is_mobile_device() {
  if (window.screen.width <= 900) {
    return true;
  } else {
    var UA = navigator.userAgent;
    return (
      /\b(BlackBerry|webOS|iPhone|IEMobile)\b/i.test(UA) ||
      /\b(Android|Windows Phone|iPad|iPod)\b/i.test(UA)
    );
  }
}

function is_mobile_uncached() {
  return check_is_mobile_device() || has_touch_screen();
}

let is_mobile_cache = is_mobile_uncached();

export default function is_mobile() {
  return is_mobile_cache;
}
