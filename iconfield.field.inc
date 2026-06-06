<?php
/**
 * @file
 * Field type, widget and formatter for the Icon Field.
 */

/**
 * Implements hook_field_info().
 */
function iconfield_field_info() {
  return array(
    'icon' => array(
      'label' => t('Icon'),
      'description' => t('This field stores icon keys.'),
      'instance_settings' => array(
        'alt_field' => 1,
      ),
      'default_widget' => 'icon_icon',
      'default_formatter' => 'icon',
    ),
  );
}

/**
 * Implements hook_field_instance_settings_form().
 */
function iconfield_field_instance_settings_form($field, $instance) {
  $form = array();
  $form['alt_field'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable <em>Alt</em> field'),
    '#default_value' => !empty($instance['settings']['alt_field']),
    '#description' => t('Let editors provide an accessible short-text description of the icon. Gets injected into SVG as title.'),
  );
  return $form;
}

/**
 * Implements hook_field_is_empty().
 */
function iconfield_field_is_empty($item, $field) {
  return empty($item['key']);
}

/**
 * Implements hook_field_widget_info().
 */
function iconfield_field_widget_info() {
  return array(
    'icon_icon' => array(
      'label' => t('Icon'),
      'field types' => array('icon'),
      'behaviors' => array(
        'multiple values' => FIELD_BEHAVIOR_DEFAULT,
        'default value' => FIELD_BEHAVIOR_NONE,
      ),
    ),
  );
}

/**
 * Implements hook_field_widget_form().
 */
function iconfield_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  $module_path = backdrop_get_path('module', 'iconfield');
  $form['#attached']['library'][] = array('system', 'backdrop.ajax');
  $form['#attached']['js'][] = $module_path . '/js/iconfield-widget.js';
  $form['#attached']['css'][] = $module_path . '/css/iconfield.css';

  if ($field['cardinality'] == 1) {
    $element_prefix = '<div class="iconfield-field-container"><strong>' . check_plain($instance['label']) . '</strong><div class="iconfield-wrapper">';
    $element_suffix = '</div></div>';
  }
  else {
    $element_prefix = '<div class="iconfield-wrapper">';
    $element_suffix = '</div>';
  }
  $element['widget_prefix'] = array(
    '#type' => 'markup',
    '#markup' => $element_prefix,
    '#weight' => -1,
  );

  $icon_key = isset($items[$delta]['key']) ? $items[$delta]['key'] : '';
  // Build a consistent id not affected by ajax updates.
  $id_parts = array(
    $instance['entity_type'],
    $instance['bundle'],
    $instance['field_name'],
    $delta,
  );
  $id_base = implode('-', $id_parts);
  $element['key'] = array(
    '#type' => 'hidden',
    '#default_value' => $icon_key,
    '#attributes' => array(
      'id' => $id_base,
      'class' => array('iconbrowser-target'),
    ),
  );
  $element['preview_wrapper'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('iconfield-preview-wrapper'),
      'id' => $id_base . '-preview-wrapper',
    ),
  );
  $show_alt_field = !empty($instance['settings']['alt_field']);
  // Make sure to reset the value if alt got disabled.
  $alt_default = '';
  if ($show_alt_field && isset($items[$delta]['alt'])) {
    $alt_default = $items[$delta]['alt'];
  }
  $element['alt'] = array(
    '#type' => 'textfield',
    '#title' => 'Alternative text',
    '#default_value' => $alt_default,
    '#access' => $show_alt_field,
    '#wrapper_attributes' => array('class' => array('alt-text')),
    '#description' => t('Short description of the icon for better accessibility. Omit if the icon is only decorative'),
  );

  $element['button_wrapper'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('button-wrapper')),
  );
  $element['button_wrapper']['remove'] = array(
    '#type' => 'markup',
    '#markup' => '<button type="button" class="button form-submit button-danger icon-remove">' . t('Remove icon') . '</button>',
  );
  $element['button_wrapper']['browser_link'] = array(
    '#type' => 'markup',
    '#markup' => theme('icon_browser_open_link', array(
      'additional_attributes' => array(
        'data-insert-id' => $id_base,
      ),
    )),
  );
  $element['widget_suffix'] = array(
    '#type' => 'markup',
    '#markup' => $element_suffix,
  );
  $element['#process'][] = '_iconfield_preview_update_process';
  return $element;
}

/**
 * Process callback for the more dynamic form parts.
 */
function _iconfield_preview_update_process($element, &$form_state, $form) {
  $parents = $element['#parents'];
  $ajax_path = 'iconfield/ajax/' . implode('/', $parents);
  $path_token = backdrop_hmac_base64($ajax_path . $form['#build_id'], backdrop_get_private_key() . backdrop_get_hash_salt());
  $element['update_item'] = array(
    '#type' => 'submit',
    '#value' => t('Update item'),
    // Following 2 lines are necessary to prevent node form validation.
    '#executes_submit_callback' => FALSE,
    '#limit_validation_errors' => array(),
    '#ajax' => array(
      'path' => $ajax_path . '/' . $path_token,
      'event' => 'click',
      'progress' => array(
        'type' => 'throbber',
        'message' => '',
      ),
    ),
    '#attributes' => array('class' => array('update-item-button', 'js-hide')),
    '#name' => 'update-' . implode('-', $parents),
    '#weight' => -1,// Push it inside widget wrapper.
  );

  $icon_key = !empty($element['key']['#default_value']) ? $element['key']['#default_value'] : '';
  if (!empty($form_state['input'])) {
    $form_input = backdrop_array_get_nested_value($form_state['input'], $parents);
    if (!empty($form_input) && isset($form_input['key'])) {
      $icon_key = $form_input['key'];
    }
  }
  $preview_icon = '';
  if (!empty($icon_key)) {
    $preview_icon = icon($icon_key, array(
      'attributes' => array('width' => 64, 'height' => 64),
    ));
  }
  $element['preview_wrapper']['preview'] = array(
    '#type' => 'markup',
    '#markup' => '<span class="iconfield-preview">' . $preview_icon . '</span>',
  );
  $element['preview_wrapper']['icon_key'] = array(
    '#type' => 'markup',
    '#markup' => '<span class="iconfield-icon-key">' . check_plain($icon_key) . '</span>',
  );
  return $element;
}

/**
 * Ajax callback for the hidden "update_item" button with path iconfield/ajax.
 */
function iconfield_ajax_insert() {
  // We do permission checks here, because in this case it's impossible before.
  if (!isset($_POST['form_build_id']) || !user_access('access content')) {
    // Note: ajax_get_form() below does the form_build_id check again, but we
    // need some extra check before loading the form.
    return;
  }
  $args = func_get_args();
  $retrieved_token = array_pop($args);
  $form_build_id = $_POST['form_build_id'];
  $ajax_path = 'iconfield/ajax/' . implode('/', $args);

  $compare_token = backdrop_hmac_base64($ajax_path . $form_build_id, backdrop_get_private_key() . backdrop_get_hash_salt());
  if ($retrieved_token != $compare_token) {
    return;
  }
  $form_parents = $args;

  list($form, $form_state, $form_id, $form_build_id, $commands) = ajax_get_form();

  // Takes care of tempstore.
  backdrop_process_form($form_id, $form, $form_state);

  $element = backdrop_array_get_nested_value($form, $form_parents);
  $selector = NULL;
  if ($element) {
    $selector = '#' . $element['preview_wrapper']['#attributes']['id'];
  }

  $commands = array();
  $commands[] = ajax_command_replace($selector, backdrop_render($element['preview_wrapper']));
  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * Implements hook_field_formatter_info().
 */
function iconfield_field_formatter_info() {
  $formatters['icon'] = array(
    'label' => t('Icon'),
    'field types' => array('icon'),
    'settings' => array(
      'icon_size' => 64,
      'icon_align' => '',
      'icon_wrapper_class' => '',
      'icon_link' => FALSE,
    ),
  );

  return $formatters;
}

/**
 * Implements hook_field_formatter_settings_form().
 */
function iconfield_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];
  $element = array();
  $element['icon_size'] = array(
    '#type' => 'number',
    '#title' => t('Icon size'),
    '#min' => 16,
    '#max' => 600,
    '#default_value' => $settings['icon_size'],
    '#required' => TRUE,
    '#field_suffix' => 'px',
  );
  $element['icon_align'] = array(
    '#type' => 'select',
    '#title' => t('Icon alignment'),
    '#empty_option' => t('No'),
    '#options' => array(
      'left' => t('left'),
      'right' => t('right'),
      'center' => t('center'),
    ),
    '#default_value' => $settings['icon_align'],
  );
  $element['icon_wrapper_class'] = array(
    '#type' => 'textfield',
    '#title' => t('Additional CSS class(es) on wrapper div'),
    '#default_value' => $settings['icon_wrapper_class'],
  );
  $element['icon_link'] = array(
    '#type' => 'checkbox',
    '#title' => 'Link icon to content',
    '#default_value' => $settings['icon_link'],
  );
  return $element;
}

/**
 * Implements hook_field_formatter_settings_summary().
 */
function iconfield_field_formatter_settings_summary($field, $instance, $view_mode) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];
  $summary = array();
  $summary[] = t('Icon size') . ': ' . $settings['icon_size'];
  $summary[] = t('Icon alignment') . ': ' . $settings['icon_align'];
  if ($settings['icon_wrapper_class']) {
    $summary[] = t('Additional CSS class(es) on wrapper div') . ': ' . check_plain($settings['icon_wrapper_class']);
  }
  $yes_no = ($settings['icon_link']) ? t('Yes') : t('No');
  $summary[] = t('Link icon to content') . ': ' . $yes_no;

  return implode('<br />', $summary);
}

/**
 * Implements hook_field_formatter_view().
 */
function iconfield_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();
  $settings = $display['settings'];

  foreach ($items as $delta => $item) {
    $icon_options = array(
      'attributes' => array(
        'width' => $settings['icon_size'],
        'height' => $settings['icon_size'],
      ),
    );
    if ($instance['settings']['alt_field'] && $item['alt']) {
      $icon_options['alt'] = check_plain($item['alt']);
    }
    $icon = icon($item['key'], $icon_options);
    if ($settings['icon_link']) {
      $uri = entity_uri($entity_type, $entity);
      $icon = l($icon, $uri['path'], array('html' => TRUE));
    }
    $classes = array('icon-wrapper');
    if ($settings['icon_align']) {
      $classes[] = 'align-' . $settings['icon_align'];
    }
    if ($settings['icon_wrapper_class']) {
      $classes[] = check_plain($settings['icon_wrapper_class']);
    }
    $element[$delta] = array('#markup' => '<div class="' . implode(' ', $classes) . '">' . $icon . '</div>');
  }

  return $element;
}
