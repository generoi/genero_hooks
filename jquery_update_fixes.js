/* eslint no-param-reassign: 0 */
(function ($) {
  'use strict';

  // Hack to fix admin_menu and jquery.ui.spinner
  // @see https://drupal.org/node/1961178
  if (!$.browser) $.browser = { msie: false };
}(jQuery));
