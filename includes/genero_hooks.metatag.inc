<?php

/**
 * Implements hook_metatag_info().
 */
function genero_hooks_metatag_info() {
  $info['tags']['article:publisher'] = array(
    'label' => t('Open Graph article publisher'),
    'description' => 'Links an article to a publishers Facebook page.',
    'class' => 'DrupalTextMetaTag',
    'group' => 'open-graph',
    'element' => array(
      '#theme' => 'metatag_opengraph',
    ),
  );
  $info['tags']['article:author'] = array(
    'label' => t('Open Graph article author'),
    'description' => 'Links an article to an authors Facebook profile.',
    'class' => 'DrupalTextMetaTag',
    'group' => 'open-graph',
    'element' => array(
      '#theme' => 'metatag_opengraph',
    ),
  );
  return $info;
}

/**
 * Implements hook_metatag_metatags_view_alter().
 */
function genero_hooks_metatag_metatags_view_alter(&$output, $instance) {
  // Use the facebook share image style for og:image
  if (!empty($output['og:image'])) {
    $image_url = $output['og:image']['#attached']['drupal_add_html_head'][0][0]['#value'];
    if (valid_url($image_url, TRUE)) {
      $image_url = _genero_hooks_metatag_share_image($image_url);
      $output['og:image'] = array(
        '#attached' => array(
          'drupal_add_html_head' => array(
            array(array(
              '#theme' => 'metatag_opengraph',
              '#tag' => 'meta',
              '#id' => 'metatag_og:image',
              '#name' => 'og:image',
              // Unfortunately AddThis does not respect this.
              '#attributes' => array('nopin' => 'nopin', 'class' => array('at_exclude')),
              '#value' => $image_url,
            ), 'metatag_og:image'),
          ),
        ),
      );
    }
  }

  // Add site slogan as the default description.
  if (empty($output['og:description']) && empty($output['description'])) {
    $output['og:description'] = array(
      '#attached' => array(
        'drupal_add_html_head' => array(
          array(array(
            '#theme' => 'metatag_opengraph',
            '#tag' => 'meta',
            '#id' => 'metatag_og:description',
            '#name' => 'og:description',
            '#value' => variable_get('site_slogan'),
          ), 'metatag_og:description'),
        ),
      ),
    );
    $output['description'] = array(
      '#attached' => array(
        'drupal_add_html_head' => array(
          array(array(
            '#theme' => 'metatag',
            '#tag' => 'meta',
            '#id' => 'metatag_description',
            '#name' => 'description',
            '#value' => variable_get('site_slogan'),
          ), 'metatag_description'),
        ),
      ),
    );
  }
  // Prevent descriptions longer than 150 characters.
  foreach (array('description', 'og:description') as $key) {
    $value = &$output[$key]['#attached']['drupal_add_html_head'][0][0]['#value'];
    if (drupal_strlen($value) > 150) {
      $value = drupal_substr($value, 0, 149) . '…';
    }
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 * Increase the maxlength of the slogan so it can be as long as descriptions.
 */
function genero_hooks_form_system_site_information_settings_alter(&$form, &$form_state, $form_id) {
  $form['site_information']['site_slogan']['#maxlength'] = 150;
}

/**
 * Return a share image style url from a regular file url.
 */
function _genero_hooks_metatag_share_image($url) {
  // image path starting from the base
  $image_path = parse_url($url, PHP_URL_PATH);
  // Relative path to public://
  $files_dir = '/' . variable_get('file_public_path', conf_path() . '/files') . '/';
  // Make sure this image is in the files directory.
  if (strpos($image_path, $files_dir) !== FALSE) {
    $image_uri = str_replace($files_dir, '', $image_path);
    // If it's an image style, find the real image.
    $image_uri = 'public://' . preg_replace('/^styles\/[^\/]+\/(public|private)\//', '', $image_uri);
    $url = image_style_url('facebook_share', $image_uri);
  }

  // Allow cache bursting.
  if (defined('OG_IMAGE_VERSION')) {
    $url .= '&v=' . OG_IMAGE_VERSION;
  }

  return $url;
}

/**
 * Implements hook_image_default_styles().
 */
function genero_hooks_image_default_styles() {
  if (module_exists('smartcrop')) {
    $effect = array(
      'name' => 'smartcrop_scale_and_crop',
      'data' => array(
        'width' => 400,
        'height' => 400,
        'upscale' => 1,
      ),
      'weight' => 0,
    );
  }
  else {
    $effect = array(
      'name' => 'image_scale_and_crop',
      'data' => array(
        'width' => 400,
        'height' => 400,
      ),
      'weight' => 0,
    );
  }

  $styles['facebook_share'] = array(
    'label' => 'Facebook share',
    'name' => 'facebook_share',
    'storage' => IMAGE_STORAGE_NORMAL,
    'effects' => array(
      $effect,
    ),
  );
}
