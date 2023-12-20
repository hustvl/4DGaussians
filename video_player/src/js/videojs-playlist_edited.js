/*! @name videojs-playlist @version 5.0.1 @license Apache-2.0 */
(function (global, factory) {
  typeof exports === 'object' && typeof module !== 'undefined' ? module.exports = factory(require('video.js')) :
  typeof define === 'function' && define.amd ? define(['video.js'], factory) :
  (global = typeof globalThis !== 'undefined' ? globalThis : global || self, global.videojsPlaylist = factory(global.videojs));
})(this, (function (videojs) { 'use strict';

  function _interopDefaultLegacy (e) { return e && typeof e === 'object' && 'default' in e ? e : { 'default': e }; }

  var videojs__default = /*#__PURE__*/_interopDefaultLegacy(videojs);

  /**
   * Validates a number of seconds to use as the auto-advance delay.
   *
   * @private
   * @param   {number} s
   *          The number to check
   *
   * @return  {boolean}
   *          Whether this is a valid second or not
   */
  const validSeconds = s => typeof s === 'number' && !isNaN(s) && s >= 0 && s < Infinity;
  /**
   * Resets the auto-advance behavior of a player.
   *
   * @param {Player} player
   *        The player to reset the behavior on
   */


  let reset = player => {
    const aa = player.playlist.autoadvance_;

    if (aa.timeout) {
      player.clearTimeout(aa.timeout);
    }

    if (aa.trigger) {
      player.off('ended', aa.trigger);
    }

    aa.timeout = null;
    aa.trigger = null;
  };
  /**
   * Sets up auto-advance behavior on a player.
   *
   * @param  {Player} player
   *         the current player
   *
   * @param  {number} delay
   *         The number of seconds to wait before each auto-advance.
   *
   * @return {undefined}
   *         Used to short circuit function logic
   */


  const setup = (player, delay) => {
    reset(player); // Before queuing up new auto-advance behavior, check if `seconds` was
    // called with a valid value.

    if (!validSeconds(delay)) {
      player.playlist.autoadvance_.delay = null;
      return;
    }

    player.playlist.autoadvance_.delay = delay;

    player.playlist.autoadvance_.trigger = function () {
      // This calls setup again, which will reset the existing auto-advance and
      // set up another auto-advance for the next "ended" event.
      const cancelOnPlay = () => setup(player, delay); // If there is a "play" event while we're waiting for an auto-advance,
      // we need to cancel the auto-advance. This could mean the user seeked
      // back into the content or restarted the content. This is reproducible
      // with an auto-advance > 0.


      player.one('play', cancelOnPlay);
      player.playlist.autoadvance_.timeout = player.setTimeout(() => {
        reset(player);
        player.off('play', cancelOnPlay);
        player.playlist.next();
      }, delay * 1000);
    };

    player.one('ended', player.playlist.autoadvance_.trigger);
  };

  /**
   * Removes all remote text tracks from a player.
   *
   * @param  {Player} player
   *         The player to clear tracks on
   */

  const clearTracks = player => {
    const tracks = player.remoteTextTracks();
    let i = tracks && tracks.length || 0; // This uses a `while` loop rather than `forEach` because the
    // `TextTrackList` object is a live DOM list (not an array).

    while (i--) {
      player.removeRemoteTextTrack(tracks[i]);
    }
  };
  /**
   * Plays an item on a player's playlist.
   *
   * @param  {Player} player
   *         The player to play the item on
   *
   * @param  {Object} item
   *         A source from the playlist.
   *
   * @return {Player}
   *         The player that is now playing the item
   */


  const playItem = (player, item) => {
    const replay = !player.paused() || player.ended();
  //  player.trigger('beforeplaylistitem', item.originalValue || item);

    if (item.playlistItemId_) {
      player.playlist.currentPlaylistItemId_ = item.playlistItemId_;
    }

    player.poster(item.poster || '');
    player.src(item.sources);
    //player.currentTime(5);
    clearTracks(player);
    player.ready(() => {
      (item.textTracks || []).forEach(player.addRemoteTextTrack.bind(player));
     // player.trigger('playlistitem', item.originalValue || item);

      if (replay) {
        const playPromise = player.play(); // silence error when a pause interrupts a play request
        // on browsers which return a promise

        if (typeof playPromise !== 'undefined' && typeof playPromise.then === 'function') {
          playPromise.then(null, e => {});
        }
      }

      setup(player, player.playlist.autoadvance_.delay);
    });
    return player;
  };

  /**
   * Returns whether a playlist item is an object of any kind, excluding null.
   *
   * @private
   *
   * @param {Object}
   *         value to be checked
   *
   * @return {boolean}
   *          The result
   */

  const isItemObject = value => {
    return !!value && typeof value === 'object';
  };
  /**
   * Look through an array of playlist items and transform any primitive
   * as well as null values to objects. This method also adds a property
   * to the transformed item containing original value passed in an input list.
   *
   * @private
   *
   * @param  {Array} arr
   *         An array of playlist items
   *
   * @return {Array}
   *         A new array with transformed items
   */


  const transformPrimitiveItems = arr => {
    const list = [];
    let tempItem;
    arr.forEach(item => {
      if (!isItemObject(item)) {
        tempItem = Object(item);
        tempItem.originalValue = item;
      } else {
        tempItem = item;
      }

      list.push(tempItem);
    });
    return list;
  };
  /**
   * Generate a unique id for each playlist item object. This id will be used to determine
   * index of an item in the playlist array for cases where there are multiple items with
   * the same source set.
   *
   * @private
   *
   * @param  {Array} arr
   *         An array of playlist items
   */


  const generatePlaylistItemId = arr => {
    let guid = 1;
    arr.forEach(item => {
      item.playlistItemId_ = guid++;
    });
  };
  /**
   * Look through an array of playlist items for a specific playlist item id.
   *
   * @private
   * @param   {Array} list
   *          An array of playlist items to look through
   *
   * @param   {number} currentItemId
   *          The current item ID.
   *
   * @return  {number}
   *          The index of the playlist item or -1 if not found
   */


  const indexInPlaylistItemIds = (list, currentItemId) => {
    for (let i = 0; i < list.length; i++) {
      if (list[i].playlistItemId_ === currentItemId) {
        return i;
      }
    }

    return -1;
  };
  /**
   * Given two sources, check to see whether the two sources are equal.
   * If both source urls have a protocol, the protocols must match, otherwise, protocols
   * are ignored.
   *
   * @private
   * @param {string|Object} source1
   *        The first source
   *
   * @param {string|Object} source2
   *        The second source
   *
   * @return {boolean}
   *         The result
   */


  const sourceEquals = (source1, source2) => {
    let src1 = source1;
    let src2 = source2;

    if (typeof source1 === 'object') {
      src1 = source1.src;
    }

    if (typeof source2 === 'object') {
      src2 = source2.src;
    }

    if (/^\/\//.test(src1)) {
      src2 = src2.slice(src2.indexOf('//'));
    }

    if (/^\/\//.test(src2)) {
      src1 = src1.slice(src1.indexOf('//'));
    }

    return src1 === src2;
  };
  /**
   * Look through an array of playlist items for a specific `source`;
   * checking both the value of elements and the value of their `src`
   * property.
   *
   * @private
   * @param   {Array} arr
   *          An array of playlist items to look through
   *
   * @param   {string} src
   *          The source to look for
   *
   * @return  {number}
   *          The index of that source or -1
   */


  const indexInSources = (arr, src) => {
    for (let i = 0; i < arr.length; i++) {
      const sources = arr[i].sources;

      if (Array.isArray(sources)) {
        for (let j = 0; j < sources.length; j++) {
          const source = sources[j];

          if (source && sourceEquals(source, src)) {
            return i;
          }
        }
      }
    }

    return -1;
  };
  /**
   * Randomize the contents of an array.
   *
   * @private
   * @param  {Array} arr
   *         An array.
   *
   * @return {Array}
   *         The same array that was passed in.
   */


  const randomize = arr => {
    let index = -1;
    const lastIndex = arr.length - 1;

    while (++index < arr.length) {
      const rand = index + Math.floor(Math.random() * (lastIndex - index + 1));
      const value = arr[rand];
      arr[rand] = arr[index];
      arr[index] = value;
    }

    return arr;
  };
  /**
   * Factory function for creating new playlist implementation on the given player.
   *
   * API summary:
   *
   * playlist(['a', 'b', 'c']) // setter
   * playlist() // getter
   * playlist.currentItem() // getter, 0
   * playlist.currentItem(1) // setter, 1
   * playlist.next() // 'c'
   * playlist.previous() // 'b'
   * playlist.first() // 'a'
   * playlist.last() // 'c'
   * playlist.autoadvance(5) // 5 second delay
   * playlist.autoadvance() // cancel autoadvance
   *
   * @param  {Player} player
   *         The current player
   *
   * @param  {Array=} initialList
   *         If given, an initial list of sources with which to populate
   *         the playlist.
   *
   * @param  {number=}  initialIndex
   *         If given, the index of the item in the list that should
   *         be loaded first. If -1, no video is loaded. If omitted, The
   *         the first video is loaded.
   *
   * @return {Function}
   *         Returns the playlist function specific to the given player.
   */


  function factory(player, initialList, initialIndex = 0) {
    let list = null;
    let changing = false;
    /**
     * Get/set the playlist for a player.
     *
     * This function is added as an own property of the player and has its
     * own methods which can be called to manipulate the internal state.
     *
     * @param  {Array} [newList]
     *         If given, a new list of sources with which to populate the
     *         playlist. Without this, the function acts as a getter.
     *
     * @param  {number}  [newIndex]
     *         If given, the index of the item in the list that should
     *         be loaded first. If -1, no video is loaded. If omitted, The
     *         the first video is loaded.
     *
     * @return {Array}
     *         The playlist
     */

    const playlist = player.playlist = (newList, newIndex = 0) => {
      if (changing) {
        throw new Error('do not call playlist() during a playlist change');
      }

      if (Array.isArray(newList)) {
        // @todo - Simplify this to `list.slice()` for v5.
       // const previousPlaylist = Array.isArray(list) ? list.slice() : null;
       // const nextPlaylist = newList.slice();
      //  list = nextPlaylist.slice(); // Transform any primitive and null values in an input list to objects
        list = newList.slice();
        // if (list.filter(item => isItemObject(item)).length !== list.length) {
        //   list = transformPrimitiveItems(list);
        // } // Add unique id to each playlist item. This id will be used
        // to determine index in cases where there are more than one
        // identical sources in the playlist.


        //generatePlaylistItemId(list); // Mark the playlist as changing during the duringplaylistchange lifecycle.

        // changing = true;
        // player.trigger({
        //   type: 'duringplaylistchange',
        //   nextIndex: newIndex,
        //   nextPlaylist,
        //   previousIndex: playlist.currentIndex_,
        //   // @todo - Simplify this to simply pass along `previousPlaylist` for v5.
        //   previousPlaylist: previousPlaylist || []
        // });
        changing = false;

        if (newIndex !== -1) {
          playlist.currentItem(newIndex);
        } // The only time the previous playlist is null is the first call to this
        // function. This allows us to fire the `duringplaylistchange` event
        // every time the playlist is populated and to maintain backward
        // compatibility by not firing the `playlistchange` event on the initial
        // population of the list.
        //
        // @todo - Remove this condition in preparation for v5.


        // if (previousPlaylist) {
        //   player.setTimeout(() => {
        //     player.trigger('playlistchange');
        //   }, 0);
        // }
      } // Always return a shallow clone of the playlist list.
      //  We also want to return originalValue if any item in the list has it.


      //return list.map(item => item.originalValue || item).slice();
    }; // On a new source, if there is no current item, disable auto-advance.


    player.on('loadstart', () => {
      if (playlist.currentItem() === -1) {
        reset(player);
      }
    });
    playlist.currentIndex_ = -1;
    playlist.player_ = player;
    playlist.autoadvance_ = {};
    playlist.repeat_ = false;
    playlist.currentPlaylistItemId_ = null;
    /**
     * Get or set the current item in the playlist.
     *
     * During the duringplaylistchange event, acts only as a getter.
     *
     * @param  {number} [index]
     *         If given as a valid value, plays the playlist item at that index.
     *
     * @return {number}
     *         The current item index.
     */

    playlist.currentItem = index => {
      // If the playlist is changing, only act as a getter.
      //console.log("index:", index);
      if (changing) {
        return playlist.currentIndex_;
      } // Act as a setter when the index is given and is a valid number.


      if (typeof index === 'number' /*&& playlist.currentIndex_ !== index */ && index >= 0 && index < list.length) {
        playlist.currentIndex_ = index;
        playItem(playlist.player_, list[playlist.currentIndex_]); // When playing multiple videos in a playlist the videojs PosterImage
        // will be hidden using CSS. However, in some browsers the native poster
        // attribute will briefly appear while the new source loads. Prevent
        // this by hiding every poster after the first play list item. This
        // doesn't cover every use case for showing/hiding the poster, but
        // it will significantly improve the user experience.

        if (index > 0) {
          player.poster('');
        }

        return playlist.currentIndex_;
      }

      const src = playlist.player_.currentSrc() || ''; // If there is a currentPlaylistItemId_, validate that it matches the
      // current source URL returned by the player. This is sufficient evidence
      // to suggest that the source was set by the playlist plugin. This code
      // exists primarily to deal with playlists where multiple items have the
      // same source.

      if (playlist.currentPlaylistItemId_) {
        const indexInItemIds = indexInPlaylistItemIds(list, playlist.currentPlaylistItemId_);
        const item = list[indexInItemIds]; // Found a match, this is our current index!

        if (item && Array.isArray(item.sources) && indexInSources([item], src) > -1) {
          playlist.currentIndex_ = indexInItemIds;
          return playlist.currentIndex_;
        } // If this does not match the current source, null it out so subsequent
        // calls can skip this step.


        playlist.currentPlaylistItemId_ = null;
      } // Finally, if we don't have a valid, current playlist item ID, we can
      // auto-detect it based on the player's current source URL.


      playlist.currentIndex_ = playlist.indexOf(src);
      return playlist.currentIndex_;
    };
    /**
     * Checks if the playlist contains a value.
     *
     * @param  {string|Object|Array} value
     *         The value to check
     *
     * @return {boolean}
     *         The result
     */


    playlist.contains = value => {
      return playlist.indexOf(value) !== -1;
    };
    /**
     * Gets the index of a value in the playlist or -1 if not found.
     *
     * @param  {string|Object|Array} value
     *         The value to find the index of
     *
     * @return {number}
     *         The index or -1
     */


    playlist.indexOf = value => {
      if (typeof value === 'string') {
        return indexInSources(list, value);
      }

      const sources = Array.isArray(value) ? value : value.sources;

      for (let i = 0; i < sources.length; i++) {
        const source = sources[i];

        if (typeof source === 'string') {
          return indexInSources(list, source);
        } else if (source.src) {
          return indexInSources(list, source.src);
        }
      }

      return -1;
    };
    /**
     * Get the index of the current item in the playlist. This is identical to
     * calling `currentItem()` with no arguments.
     *
     * @return {number}
     *         The current item index.
     */


    playlist.currentIndex = () => playlist.currentItem();
    /**
     * Get the index of the last item in the playlist.
     *
     * @return {number}
     *         The index of the last item in the playlist or -1 if there are no
     *         items.
     */


    playlist.lastIndex = () => list.length - 1;
    /**
     * Get the index of the next item in the playlist.
     *
     * @return {number}
     *         The index of the next item in the playlist or -1 if there is no
     *         current item.
     */


    playlist.nextIndex = () => {
      const current = playlist.currentItem();

      if (current === -1) {
        return -1;
      }

      const lastIndex = playlist.lastIndex(); // When repeating, loop back to the beginning on the last item.

      if (playlist.repeat_ && current === lastIndex) {
        return 0;
      } // Don't go past the end of the playlist.


      return Math.min(current + 1, lastIndex);
    };
    /**
     * Get the index of the previous item in the playlist.
     *
     * @return {number}
     *         The index of the previous item in the playlist or -1 if there is
     *         no current item.
     */


    playlist.previousIndex = () => {
      const current = playlist.currentItem();

      if (current === -1) {
        return -1;
      } // When repeating, loop back to the end of the playlist.


      if (playlist.repeat_ && current === 0) {
        return playlist.lastIndex();
      } // Don't go past the beginning of the playlist.


      return Math.max(current - 1, 0);
    };
    /**
     * Plays the first item in the playlist.
     *
     * @return {Object|undefined}
     *         Returns undefined and has no side effects if the list is empty.
     */


    playlist.first = () => {
      if (changing) {
        return;
      }

      const newItem = playlist.currentItem(0);

      if (list.length) {
        return list[newItem].originalValue || list[newItem];
      }

      playlist.currentIndex_ = -1;
    };
    /**
     * Plays the last item in the playlist.
     *
     * @return {Object|undefined}
     *         Returns undefined and has no side effects if the list is empty.
     */


    playlist.last = () => {
      if (changing) {
        return;
      }

      const newItem = playlist.currentItem(playlist.lastIndex());

      if (list.length) {
        return list[newItem].originalValue || list[newItem];
      }

      playlist.currentIndex_ = -1;
    };
    /**
     * Plays the next item in the playlist.
     *
     * @return {Object|undefined}
     *         Returns undefined and has no side effects if on last item.
     */


    playlist.next = () => {
      if (changing) {
        return;
      }

      const index = playlist.nextIndex();

      if (index !== playlist.currentIndex_) {
        const newItem = playlist.currentItem(index);
        return list[newItem].originalValue || list[newItem];
      }
    };
    /**
     * Plays the previous item in the playlist.
     *
     * @return {Object|undefined}
     *         Returns undefined and has no side effects if on first item.
     */


    playlist.previous = () => {
      if (changing) {
        return;
      }

      const index = playlist.previousIndex();

      if (index !== playlist.currentIndex_) {
        const newItem = playlist.currentItem(index);
        return list[newItem].originalValue || list[newItem];
      }
    };
    /**
     * Set up auto-advance on the playlist.
     *
     * @param  {number} [delay]
     *         The number of seconds to wait before each auto-advance.
     */


    playlist.autoadvance = delay => {
      setup(playlist.player_, delay);
    };
    /**
     * Sets `repeat` option, which makes the "next" video of the last video in
     * the playlist be the first video in the playlist.
     *
     * @param  {boolean} [val]
     *         The value to set repeat to
     *
     * @return {boolean}
     *         The current value of repeat
     */


    playlist.repeat = val => {
      if (val === undefined) {
        return playlist.repeat_;
      }

      if (typeof val !== 'boolean') {
        videojs__default["default"].log.error('videojs-playlist: Invalid value for repeat', val);
        return;
      }

      playlist.repeat_ = !!val;
      return playlist.repeat_;
    };
    /**
     * Sorts the playlist array.
     *
     * @see {@link https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/sort}
     * @fires playlistsorted
     *
     * @param {Function} compare
     *        A comparator function as per the native Array method.
     */


    playlist.sort = compare => {
      // Bail if the array is empty.
      if (!list.length) {
        return;
      }

      list.sort(compare); // If the playlist is changing, don't trigger events.

      if (changing) {
        return;
      }
      /**
       * Triggered after the playlist is sorted internally.
       *
       * @event playlistsorted
       * @type {Object}
       */


      player.trigger('playlistsorted');
    };
    /**
     * Reverses the playlist array.
     *
     * @see {@link https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/reverse}
     * @fires playlistsorted
     */


    playlist.reverse = () => {
      // Bail if the array is empty.
      if (!list.length) {
        return;
      }

      list.reverse(); // If the playlist is changing, don't trigger events.

      if (changing) {
        return;
      }
      /**
       * Triggered after the playlist is sorted internally.
       *
       * @event playlistsorted
       * @type {Object}
       */


      player.trigger('playlistsorted');
    };
    /**
     * Shuffle the contents of the list randomly.
     *
     * @see   {@link https://github.com/lodash/lodash/blob/40e096b6d5291a025e365a0f4c010d9a0efb9a69/shuffle.js}
     * @fires playlistsorted
     * @todo  Make the `rest` option default to `true` in v5.0.0.
     * @param {Object} [options]
     *        An object containing shuffle options.
     *
     * @param {boolean} [options.rest = false]
     *        By default, the entire playlist is randomized. However, this may
     *        not be desirable in all cases, such as when a user is already
     *        watching a video.
     *
     *        When `true` is passed for this option, it will only shuffle
     *        playlist items after the current item. For example, when on the
     *        first item, will shuffle the second item and beyond.
     */


    playlist.shuffle = ({
      rest
    } = {}) => {
      let index = 0;
      let arr = list; // When options.rest is true, start randomization at the item after the
      // current item.

      if (rest) {
        index = playlist.currentIndex_ + 1;
        arr = list.slice(index);
      } // Bail if the array is empty or too short to shuffle.


      if (arr.length <= 1) {
        return;
      }

      randomize(arr); // When options.rest is true, splice the randomized sub-array back into
      // the original array.

      if (rest) {
        list.splice(...[index, arr.length].concat(arr));
      } // If the playlist is changing, don't trigger events.


      if (changing) {
        return;
      }
      /**
       * Triggered after the playlist is sorted internally.
       *
       * @event playlistsorted
       * @type {Object}
       */


      player.trigger('playlistsorted');
    }; // If an initial list was given, populate the playlist with it.


    if (Array.isArray(initialList)) {
      playlist(initialList.slice(), initialIndex); // If there is no initial list given, silently set an empty array.
    } else {
      list = [];
    }

    return playlist;
  }

  var version = "5.0.1";

  const registerPlugin = videojs__default["default"].registerPlugin || videojs__default["default"].plugin;
  /**
   * The video.js playlist plugin. Invokes the playlist-maker to create a
   * playlist function on the specific player.
   *
   * @param {Array} list
   *        a list of sources
   *
   * @param {number} item
   *        The index to start at
   */

  const plugin = function (list, item) {
    factory(this, list, item);
  };

  registerPlugin('playlist', plugin);
  plugin.VERSION = version;

  return plugin;

}));
