<?php

/**
 * Implements hook_metatag_pattern_alter().
 *
 * Localize metatags on localized taxonomy terms.
 */
function genero_hooks_metatag_pattern_alter(&$pattern, &$types, $tag_name) {
  if (!isset($types['term'])) {
    return;
  }
  $term = $types['term'];
  if (!module_exists('i18n_taxonomy') || i18n_taxonomy_vocabulary_mode($term->vid) != I18N_MODE_LOCALIZE) {
    return;
  }
  $pattern = i18n_string(array('taxonomy_metatags', $term->tid, $tag_name), $pattern);
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
              '#theme' => 'metatag',
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
            '#theme' => 'metatag',
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
  return $styles;
}

/**
 * Implements hook__form_FORM_ID_alter().
 */
function genero_hooks_form_taxonomy_form_term_alter(&$form) {
  // if (!module_exists('i18n_taxonomy') || i18n_taxonomy_vocabulary_mode($term->vid) != I18N_MODE_LOCALIZE) {
  //   return;
  // }
  array_unshift($form['#submit'], 'genero_hooks_taxonomy_form_term_pre_submit');
  $form['#submit'][] = 'genero_hooks_taxonomy_form_term_post_submit';
}

/**
 * Metatag fields which are translated.
 */
function genero_hooks_taxonomy_term_translated_metatags() {
  return array('og:title', 'og:description', 'title', 'description');
}

/**
 * Handler at the beginning of the taxonomy_form_term submit process.
 */
function genero_hooks_taxonomy_form_term_pre_submit($form, &$form_state) {
  $vid = $form['#vocabulary']->vid;
  if (!module_exists('i18n_taxonomy') || i18n_taxonomy_vocabulary_mode($vid) != I18N_MODE_LOCALIZE) {
    return;
  }
  if (empty($form_state['values']['metatags'])) {
    return;
  }

  $form_state['genero_hooks_taxonomy_metatags'] = array();
  $langcode = $form_state['values']['language'];
  if (!empty($form_state['values']['metatags'][$langcode])) {
    foreach (genero_hooks_taxonomy_term_translated_metatags() as $key) {
      if (isset($form_state['values']['metatags'][$langcode])) {
        // Store the metatag keys and values separatenly as default values are
        // removed by the metatag modules submit handler.
        $form_state['genero_hooks_taxonomy_metatags'][$key] = $form_state['values']['metatags'][$langcode][$key];
      }
    }
  }
}

/**
 * Submit handler at the end of taxonomy_form_term submit process.
 */
function genero_hooks_taxonomy_form_term_post_submit($form, &$form_state) {
  // Term now contains the tid.
  $term = $form_state['term'];
  if (!empty($form_state['genero_hooks_taxonomy_metatags'])) {
    foreach (genero_hooks_taxonomy_term_translated_metatags() as $key) {
      $value = $form_state['genero_hooks_taxonomy_metatags'][$key]['value'];
      i18n_string_update(array('taxonomy_metatags', $term->tid, $key), $value);
    }
  }
}

/**
 * Implements hook_taxonomy_term_delete().
 *
 * Remove the localized metatags from deleted taxonomy terms.
 */
function genero_hooks_taxonomy_term_delete($term) {
  if (!module_exists('i18n_taxonomy') || i18n_taxonomy_vocabulary_mode($term->vid) != I18N_MODE_LOCALIZE) {
    return;
  }
  foreach (genero_hooks_taxonomy_term_translated_metatags() as $key) {
    i18n_string_remove("taxonomy_metatags:$term->tid:$key");
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Provide localized metatags for localized taxonomy terms.
 */
function genero_hooks_form_i18n_string_translate_page_form_alter(&$form, &$form_state) {
  if (!isset($form['term'])) {
    return;
  }
  $tid = $form['term']['#value'];
  $term = taxonomy_term_load($tid);
  $langcode = $form['langcode']['#value'];
  $instance = "taxonomy_term:$term->vocabulary_machine_name";
  $options = array();
  // Localized terms always has the language set to LANGUAGE_NONE
  if (module_exists('i18n_taxonomy') && i18n_taxonomy_vocabulary_mode($term->vid) == I18N_MODE_LOCALIZE) {
    $metatags = !empty($term->metatags[$term->language]) ? $term->metatags[$term->language] : metatag_config_load_with_defaults($instance);
    $options['context'] = 'taxonomy_term';
    $options['token types'] = array(token_get_entity_mapping('entity', 'taxonomy_term'));
    metatag_metatags_form($form, $instance, $metatags, $options);
    // Move metatags above submit button (weight 10).
    $form['metatags']['#weight'] = 9;
    $allowed_metatags = genero_hooks_taxonomy_term_translated_metatags();
    // Hide the fields which we are not allowed to translate.
    foreach (element_children($form['metatags'][$term->language]) as $key) {
      $metatag = &$form['metatags'][$term->language][$key];
      // Fieldset
      if (isset($metatag['#type']) && $metatag['#type'] == 'fieldset') {
        foreach (element_children($metatags) as $sub_key) {
          // Hide tags not allowed
          if (!in_array($sub_key, $allowed_metatags)) {
            $metatag[$sub_key]['#access'] = FALSE;
          }
          // Set to translated value
          else {
            $value = $metatag[$sub_key]['value']['#default_value'];
            $metatag[$sub_key]['value']['#default_value'] = i18n_string(array('taxonomy_metatags', $tid, $sub_key), $value);
          }
        }
      }
      // Direct
      else {
        // Hide tags not allowed
        if (!in_array($key, $allowed_metatags)) {
          $metatag['#access'] = FALSE;
        }
        // Set to translated value
        else {
          $value = $metatag['value']['#default_value'];
          $metatag['value']['#default_value'] = i18n_string(array('taxonomy_metatags', $tid, $key), $value);
        }
      }
    }
  }
  $form['#submit'][] = 'genero_hooks_form_i18n_string_translate_page_form_submit';
}

/**
 * Submit handler for when the localized term translation page is saved.
 */
function genero_hooks_form_i18n_string_translate_page_form_submit($form, &$form_state) {
  $tid = $form['term']['#value'];
  $langcode = $form['langcode']['#value'];

  if (!empty($form_state['values']['metatags'][LANGUAGE_NONE])) {
    $metatags = &$form_state['values']['metatags'][LANGUAGE_NONE];
    // @TODO remove values which are set to the default
    //
    // Unset meta tag values that equal their default values.
    // foreach ($form_state['values']['metatags'] as $langcode => $values) {
    //   if (!empty($form['metatags'][$langcode]['#metatag_defaults'])) {
    //     metatag_filter_values_from_defaults($form_state['values']['metatags'][$langcode], $form['metatags'][$langcode]['#metatag_defaults']);
    //   }
    // }

    // Update the i18n_string translation of each allowed metatag.
    // @see i18n_string_translate_page_form_submit().
    $count = $success = 0;
    foreach (genero_hooks_taxonomy_term_translated_metatags() as $key) {
      if (!empty($metatags[$key])) {
        $value = $metatags[$key];
        $count++;
        list($textgroup, $context) = i18n_string_context(array('taxonomy_metatags', $tid, $key));
        $result = i18n_string_textgroup($textgroup)->update_translation($context, $langcode, $value);
        $success += ($result ? 1 : 0);
      }
    }
    if ($success) {
      drupal_set_message(format_plural($success, 'A translation was saved successfully.', '@count translations were saved successfully.'));
    }
    if ($error = $count - $success) {
      drupal_set_message(format_plural($error, 'A translation could not be saved.', '@count translations could not be saved.'), 'warning');
    }
  }
}


/**
 * Implements hook_i18n_string_info().
 */
function genero_hooks_i18n_string_info() {
  $groups['taxonomy_metatags'] = array(
    'title' => t('Taxonomy Metatags'),
    'description' => t('Localized taxonomy term metatags. Provided by genero_hooks.'),
    'format' => FALSE,
    'list' => TRUE,
  );
  return $groups;
}

/**
 * Implements hook_i18n_object_info().
 */
function genero_hooks_i18n_object_info() {
  $info['taxonomy_metatags'] = array(
    'title' => t('Taxonomy Metatags category'),
    'key' => 'tid',
    'placeholders' => array(
      '%taxonomy_term' => 'tid',
    ),
    'edit path' => 'taxonomy/term/%taxonomy_term/edit',
    'string translation' => array(
      'textgroup' => 'taxonomy_metatags',
      'type' => 'term',
      'properties' => array(),
      'translate path' => 'taxonomy/term/%taxonomy_term/translate',
    )
  );
  // We only provide support for a few metatags.
  $properties = genero_hooks_taxonomy_term_translated_metatags();
  $info['taxonomy_metatags']['string translation']['properties'] = array_combine($properties, $properties);

  return $info;
}
