import videojs from '/src/js/video.js';
// import {version as VERSION} from '../package.json';

const Plugin = videojs.getPlugin('plugin');
const Button = videojs.getComponent('button');

// Default options for the plugin.
const defaults = {};

class RotateButton extends Button {
  /**
   * Create rotate button.
   *
   * @param  {Player} player
   *         A Video.js Player instance.
   *
   * @param  {Object} [options]
   *         An optional options object.
   *
   *         While not a core part of the Video.js plugin architecture, a
   *         second argument of options is a convenient way to accept inputs
   *         from your plugin's caller.
   */
  constructor(player, options) {
    super(player, options);
    this._currentRotateDeg = 0;
    this.controlText('Rotate');
  }

  buildCSSClass() {
    return 'vjs-control vjs-button rotate-0';
  }

  handleClick() {
    this.removeClass(`rotate-${this._currentRotateDeg}`);
    const tmpRotateDeg = this._currentRotateDeg + 90;

    const zoom = tmpRotateDeg % 180 === 0 ? 1 : 0.5;

    if (tmpRotateDeg % 360 === 0) {
      this._currentRotateDeg = 0;
    }

    this._currentRotateDeg = tmpRotateDeg % 360 === 0 ? 0 : tmpRotateDeg;
    this.player().rotatePlayerPlugin().rotate({ rotate: this._currentRotateDeg, zoom });
    this.addClass(`rotate-${this._currentRotateDeg}`);
  }
}

/**
 * An advanced Video.js plugin. For more information on the API
 *
 * See: https://blog.videojs.com/feature-spotlight-advanced-plugins/
 */
class RotatePlayerPlugin extends Plugin {

  /**
   * Create a RotatePlayerPlugin plugin instance.
   *
   * @param  {Player} player
   *         A Video.js Player instance.
   *
   * @param  {Object} [options]
   *         An optional options object.
   *
   *         While not a core part of the Video.js plugin architecture, a
   *         second argument of options is a convenient way to accept inputs
   *         from your plugin's caller.
   */
  constructor(player, options) {
    // the parent class will add player under this.player
    super(player);

    this.options = videojs.mergeOptions(defaults, options);

    // Browser support rotate css property.
    this._prop = null;

    this.player.ready(() => {
      this.player.addClass('vjs-rotate-player-plugin');
      this.findSupportTransformProperty();
      this.player.getChild('controlBar').addChild('rotatePlayerButton');
    });
  }

  /**
   * Find current browser supported transform css property.
   */
  findSupportTransformProperty() {
    const player = this.player;
    const properties = [
      'transform',
      'WebkitTransform',
      'MozTransform',
      'msTransform',
      'OTransform'
    ];

    this._prop = properties[0];
    if (typeof player.style !== 'undefined') {
      for (const property of properties) {
        if (typeof player.style[property] !== 'undefined') {
          this._prop = property;
          break;
        }
      }
    }
  }

  rotate(options) {
    const targetElement = this.player.el();
    const videoElement = targetElement.getElementsByClassName('vjs-tech')[0];
    const posterElement = targetElement.getElementsByClassName('vjs-poster')[0];

    targetElement.style.overflow = 'hidden';
    videoElement.style[this._prop] = `scale(${options.zoom}) rotate(${options.rotate}deg)`;
    posterElement.style[this._prop] = `scale(${options.zoom}) rotate(${options.rotate}deg)`;
  }
}

// Define default values for the plugin's `state` object here.
RotatePlayerPlugin.defaultState = {};

// Include the version number.
// RotatePlayerPlugin.VERSION = VERSION;

// Register button.
videojs.registerComponent('rotatePlayerButton', RotateButton);

// Register the plugin with video.js.
videojs.registerPlugin('rotatePlayerPlugin', RotatePlayerPlugin);

export default RotatePlayerPlugin;
