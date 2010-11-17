<?php
// $Id$

/**
 * Product Component
 */

/**
 * Implementation of hook_theme().
 * Module-specific
 * Implementation of _webform_theme_component().
 * This allows each Webform component to add information into hook_theme().
 */
function _webform_theme_product() {
  return array(
    'uc_webform_display_product' => array(
      'arguments' => array('element' => NULL),
    ),
    'uc_webform_render_product' => array(
      'arguments' => array('product' => NULL),
    ),
  );
}

/**
 * Format the text output for the product component.
 */
function theme_uc_webform_display_product($element) {
  return $element['#value'][0];
}

/**
 * Theme the the display of an individual product when rendered in a webform.
 */
function theme_uc_webform_render_product($product) {

  $output = '<span>' . $product['title'] . ', ' . t('Price:') . ' ' . variable_get('uc_currency_sign', '$') . $product['price'] . '</span>';

  return $output;
}

/**
 * Implementation of _webform_defaults_component().
 */
function _webform_defaults_product() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'email' => 1,
    'mandatory' => 0,
    'pid' => 0,
    'weight' => 0,
    'extra' => array(
      'description' => '',
    ),
  );
}

/**
 * Generate the form for editing a component.
 */
function _webform_edit_product($component) {
  $form = array();

  $form['value'] = array(
    '#type' => 'textfield',
    '#title' => t('Default Quantity'),
    '#default_value' => $component['value'],
    '#description' => t('The default quantity of product.'),
    '#size' => 5,
    '#maxlength' => 10,
    '#weight' => 0,
  );
  $form['extra']['product'] = array(
    '#type' => 'textfield',
    '#title' => t('Product'),
    '#default_value' => $component['extra']['product'],
    '#weight' => -3,
    '#size' => 60,
    '#description' => t('Please select a product. Only products that do not contain attributes will be displayed.'),
    '#autocomplete_path' => 'uc_webform/autocomplete',
  );
  $form['display']['width'] = array(
    '#type' => 'textfield',
    '#title' => t('Width'),
    '#description' => t('Width of the textfield.') . ' ' . t('Leaving blank will use the default size.'),
    '#size' => 5,
    '#maxlength' => 10,
    '#weight' => 0,
    '#parents' => array('extra', 'width'),
  );
  $form['display']['disabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Disabled'),
    '#return_value' => 1,
    '#description' => t('Make this field non-editable. Useful for setting an unchangeable default value.'),
    '#weight' => 3,
    '#default_value' => $component['extra']['disabled'],
    '#parents' => array('extra', 'disabled'),
  );

  if (isset($component['extra']['width'])) {
    $form['display']['width']['#default_value'] = $component['extra']['width'];
  }

  return $form;
}

/**
 * Render a Webform component to be part of a form.
 */
function _webform_render_product($component, $value = NULL) {

  $stock_description = "";
  $product_info = explode('_', $component['extra']['product'], 2);
  $node = node_load($product_info[0]);
  $sku = $product_info[1];
  $stock_level = uc_stock_level($sku);

  // Check stock levels. The product is only selectable if it is in stock.
  if (($stock_level === FALSE) or (intval($stock_level) > 0)) {
    // Product is available.
    $product = array('title' => check_plain($node->title), 'price' => round($node->sell_price, 2));

    $element = array(
      '#type' => 'textfield',
      '#title' => $component['name'],
      '#title_display' => $component['extra']['title_display'] ? $component['extra']['title_display'] : 'before',
      '#weight' => $component['weight'],
      '#description' => $component['extra']['description'],
      '#default_value' => isset($value) ? check_plain($value[0]) : check_plain($component['value']),
      '#field_prefix' => t('Quantity:') . ' ',
      '#field_suffix' => theme('uc_webform_render_product', $product),
      '#required' => $component['mandatory'],
      '#pre_render' => array('webform_element_title_display'),
      '#disabled' => $component['extra']['disabled'],
      '#sku' => $sku,
      '#element_validate' => array('_webform_render_product_validate'),
    );

      // This is so that products whose quantity is disabled are added to the cart correctly.
      if ($component['extra']['disabled']) {
        $element['#value'] = isset($value) ? check_plain($value[0]) : check_plain($component['value']);
      }
      if (isset($component['extra']['width'])) {
        $element['#size'] = $component['extra']['width'];
      }
  }
  else {
    // Product is out of stock
    $stock_description .= check_plain($node->title) . ' ' . t('is out of stock.') . '<br />';

    $element = array(
      '#type' => 'item',
      '#title' => $component['name'],
      '#default_value' => '',
      '#value' => '',
      '#description' => $stock_description . $component['extra']['description'],
    );
  }

  return $element;
}

/**
 * Validate product entry
 */
function _webform_render_product_validate($element, &$form_state) {

  // If the user entered a value, make sure that it's a good one.
  if (!empty($element['#value'])) {
    $match = preg_match('/\A[0-9]+\Z/', $element['#value']);
    if (($match == 0) || $match == FALSE) {
      form_error($element, t('Please enter a positive number.'));
    }
  }

  // Check to see that we have enough in stock.
  $stock = uc_stock_level($element['#sku']);
  if (($stock !== FALSE) && ($stock < $element['#value'])) {
    form_error($element, t("Only !stock of SKU: !sku remain. Please enter a different amount.", array('!stock' => $stock, '!sku' => $element['#sku'])));
  }
}

/**
 * Display the result of a submission for a component.
 */
function _webform_display_product($component, $value, $format = 'html') {

  $product_info = explode('_', $component['extra']['product'], 2);

  $node = node_load($product_info[0]);
  $result_info = array(t('Title:') . ' ' . $node->title . '<br/>' . t('SKU:') . ' ' . $product_info[1] . '<br/>' . t('Quantity:') . ' ' . $value[2]);

  return array(
    '#title' => $component['name'],
    '#weight' => $component['weight'],
    '#theme' => 'uc_webform_display_product',
    '#theme_wrappers' => $format == 'html' ? array('webform_element') : array('webform_element_text'),
    '#post_render' => array('webform_element_wrapper'),
    '#component' => $component,
    '#format' => $format,
    '#value' => $result_info,
  );
}

/**
 * Implementation of _webform_submit_component().
 */
function _webform_submit_product($component, $value) {
  $return[0] = 'product';
  if (!empty($value)) {
    // Save the nid of the product
    $return[1] = $component['extra']['product'];
    // Save the quantity
    $return[2] = $value;

    return $return;
  }
  else {
    // Save the nid of the product
    $return[1] = $component['extra']['product'];
    // Save the quantity
    $return[2] = 0;

    return $return;
  }
}

/**
 * Implementation of _webform_analysis_component().
 *
 */
function _webform_analysis_product($component, $sids = array()) {

  $results = db_query("SELECT data, no FROM {webform_submitted_data} WHERE nid = %d AND cid = %d ", $component['nid'], $component['cid']);

  $total = 0;
  while ($result = db_fetch_array($results)) {
    if ($result['no'] == 2) {
      $total += $result['data'];
    }
  }
  $product_info = explode('_', $component['extra']['product'], 2);

  $product_node = node_load($product_info[0]);

  $rows[0] = array(t($product_info[1]), $total);

  return $rows;
}

/**
 * Implementation of _webform_table_component().
 */
function _webform_table_product($component, $value) {
  return check_plain(empty($value[2]) ? '' : $value[2]);
}

/**
 * Implementation of _webform_csv_headers_component().
 */
function _webform_csv_headers_product($component, $export_options) {
  $header = array();
  $product_info = explode('_', $component['extra']['product'], 2);

  $header[0] = '';
  $header[1] = $component['name'];
  $header[2] = $product_info[1] . ' ' . t('Quantity');
  return $header;
}

/**
 * Implementation of _webform_csv_data_component().
 */
function _webform_csv_data_product($component, $export_options, $value) {
  return !isset($value[2]) ? '' : $value[2];
}