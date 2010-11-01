<?php
// $Id$

/**
 * Implementation of _webform_defaults_component().
 */
function _webform_defaults_product_list() {
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
 * Implementation of hook_theme().
 * This implementation is module-specfic, though.
 * Implementation of _webform_theme_component().
 * This allows each Webform component to add information into hook_theme().
 */
function _webform_theme_product_list() {
  return array(
    'webform_display_product_list' => array(
      'arguments' => array('element' => NULL),
    ),
  );
}

/**
 * Generate the form for editing a component.
 */
function _webform_edit_product_list($component) {
  $products = array();
  $form = array();
  $product_types = uc_product_types();

  // I need to limit the users selection to only those products that do *not* contain attributes.
  $result = db_query("SELECT n.nid, n.title, p.model FROM {node} AS n INNER JOIN {uc_products} AS p ON n.nid = p.nid WHERE n.nid NOT IN (SELECT nid FROM {uc_product_attributes}) ORDER BY n.title");

  while ($product = db_fetch_object($result)) {
    $products[$product->nid . '_' . $product->model] = check_plain($product->title);
  }

  // Most options are stored in the "extra" array, which stores any settings unique to a particular component type.
  $form['extra']['products'] = array(
    '#type' => 'select',
    '#title' => t('Products'),
    '#default_value' => $component['extra']['products'],
    '#multiple' => TRUE,
    '#description' => t('Please select your products. Only products without attributes are displayed.'),
    '#weight' => -3,
    '#size' => 20,
    '#required' => TRUE,
    '#options' => $products,
  );
  $form['extra']['multiple'] = array(
    '#type' => 'checkbox',
    '#title' => t('Multiple'),
    '#default_value' => $component['extra']['multiple'],
    '#description' => t('Check this option if the user is allowed to select multiple products.'),
    '#weight' => -2,
  );

  return $form;
}

/**
 * Render a Webform component to be part of a form.
 *
 */
function _webform_render_product_list($component, $value = NULL) {

  $products = array();
  $stock_description = "";

  foreach ($component['extra']['products'] as $val) {
    $product_info = explode('_', $val, 2);
    $node = node_load($product_info[0]);
    $stock_level = uc_stock_level($product_info[1]);

    // Check stock levels. The product is only selectable if it is in stock.
    if (($stock_level === FALSE) or (intval($stock_level) > 0)) {
      $product['title'] = check_plain($node->title);
      $product['price'] = variable_get('uc_currency_sign', '$') . round($node->sell_price, 2);
      $products[$val] = $product;
    }
    else {
      $stock_description .= check_plain($node->title) . ' ' . t('is out of stock.') . '<br />';
    }
  }

  if ($component['extra']['multiple'] == 0) {
    $form_item = array(
      '#type' => 'radios',
      '#title' => $component['name'],
      '#weight' => $component['weight'],
      '#description' => '<br />' . $stock_description . _webform_filter_descriptions($component['extra']['description']),
      '#prefix' => '<div class="webform-component-'. $component['type'] .'" id="webform-component-'. $component['form_key'] .'">',
      '#suffix' => '</div>',
      '#required' => $component['mandatory'],
      '#options' => theme('uc_webform_product_list', $products),
      //'#element_validate' => array('_webform_render_product_list_validate'),
    );
  }
  elseif ($component['extra']['multiple'] == 1) {
    $form_item = array(
      '#type' => 'checkboxes',
      '#title' => $component['name'],
      '#weight' => $component['weight'],
      '#description' => '<br />' . $stock_description . _webform_filter_descriptions($component['extra']['description']),
      '#prefix' => '<div class="webform-component-'. $component['type'] .'" id="webform-component-'. $component['form_key'] .'">',
      '#suffix' => '</div>',
      '#required' => $component['mandatory'],
      '#options' => theme('uc_webform_product_list', $products),
      //'#element_validate' => array('_webform_render_product_list_validate'),
    );
  }

  if (isset($value)) {
    $form_item['#default_value'] = check_plain($value);
  }

  return $form_item;
}

/**
 * Validate product entry
 */
function _webform_render_product_list_validate($element, &$form_state) {

  // Check to see that we have enough in stock.
  $stock = uc_stock_level($element['#sku']);
  if (($stock !== FALSE) && ($stock < $element['#value'])) {
    form_error($element, t("Only !stock of SKU: !sku remain. Please enter a different amount.", array('!stock' => $stock, '!sku' => $element['#sku'])));
  }
}


/**
 * Theme function for a product when it is rendered as a part of a list in a form.
 *
 */
function theme_uc_webform_product_list($products) {
  $output = array();

  foreach ($products as $key => $product) {
    $output[$key] = '<span>' . $product['title'] . ', ' . t('Price:') . ' ' . $product['price'] . '</span>';
  }
  return $output;
}

/**
 * Display the result of a submission for a component.
 *
 */
function _webform_display_product_list($component, $value, $format = 'html') {

  $products = array();

  if (isset($value)) {
    foreach ($value as $product) {
      if ($product != 0) {
        $node = node_load($product);
        $product_title = $node->title . ', ' . t('SKU:') . $node->model;
        array_push($products, $product_title);
      }
    }
  }

  return array(
    '#title' => $component['name'],
    '#weight' => $component['weight'],
    '#theme' => 'webform_display_product_list',
    '#theme_wrappers' => $format == 'html' ? array('webform_element') : array('webform_element_text'),
    '#post_render' => array('webform_element_wrapper'),
    '#component' => $component,
    '#format' => $format,
    '#value' => $products,
  );
}

/**
 * Format the text output for the product_list component.
 */
function theme_webform_display_product_list($element) {

  if (isset($element['#value'])) {
    $output = (count($element['#value']) > 1) ? theme('item_list', $element['#value']) : $element['#value'][0];
  }

  return $output;
}

/**
 * Implementation of _webform_submit_component().
 */
function _webform_submit_product_list($component, $value) {
  switch (gettype($value)) {
    case 'string':
      // Value came from a radio select list
      $return[0] = 'radio_product_list';
      $return[1] = $value;
      break;
    case 'array':
      // Value came from a checkboxes select list
      $return[0] = 'checkboxes_product_list';
      $count = 1;
      foreach ($value as $product) {
        if ($product != 0) {
          $return[$count] = $product;
          $count++;
        }
      }
      break;
  }

  return $return;
}

/**
 * Calculate and returns statistics about results for this component.
 *
 * This is mostly useless right now because it doesn't filter for completed checkouts.
 */
function _webform_analysis_product_list($component, $sids = array(), $single = FALSE) {

  $options = _uc_webform_products($component);
  $show_other_results = $single;

  $sid_placeholders = count($sids) ? array_fill(0, count($sids), "'%s'") : array();
  $sid_filter = count($sids) ? " AND sid IN (" . implode(",", $sid_placeholders) . ")" : "";

  $option_operator = $show_other_results ? 'NOT IN' : 'IN';
  $placeholders = count($options) ? array_fill(0, count($options), "'%s'") : array();
  $query = 'SELECT data, count(data) as datacount ' .
    ' FROM {webform_submitted_data} ' .
    ' WHERE nid = %d ' .
    ' AND cid = %d ' .
    " AND data != ''" . $sid_filter .
    ($placeholders ? ' AND data ' . $option_operator . ' (' . implode(',', $placeholders) . ')' : '') .
    ' GROUP BY data ';

  $count_query = 'SELECT count(*) as datacount ' .
    ' FROM {webform_submitted_data} ' .
    ' WHERE nid = %d ' .
    ' AND cid = %d ' .
    " AND data != ''" . $sid_filter;

  $result = db_query($query, array_merge(array($component['nid'], $component['cid']), $sids, array_keys($options)));
  $rows = array();
  $normal_count = 0;
  while ($data = db_fetch_array($result)) {
    $display_option = $single ? $data['data'] : $options[$data['data']];
    $rows[$data['data']] = array(_webform_filter_xss($display_option), $data['datacount']);
    $normal_count += $data['datacount'];
  }

  if (!$show_other_results) {
    // Order the results according to the normal options array.
    $ordered_rows = array();
    foreach (array_intersect_key($options, $rows) as $key => $label) {
      $ordered_rows[] = $rows[$key];
    }

    $rows = $ordered_rows;
  }

  return $rows;
}

/**
 * Return the result of a component value for display in a table.
 */
function _webform_table_product_list($component, $value) {

  $results = "";

  if (isset($value)) {
    foreach ($value as $product) {
      $node = node_load($product);
      $results .= "$node->model<br />";
    }
  }

  return $results;
}

/**
 * Return the header for this component to be displayed in a CSV file.
 */
function _webform_csv_headers_product_list($component, $export_options) {
  $headers = array(
    0 => array(),
    1 => array(),
    2 => array(),
  );

  $options = _uc_webform_products($component);

  if ($component['extra']['multiple'] && $export_options['select_format'] == 'separate') {
    $headers[0][] = '';
    $headers[1][] = $component['name'];

    $count = 0;
    foreach ($options as $value) {
      if ($count != 0) {
        // Empty column per sub-field in main header.
        $headers[0][] = '';
        $headers[1][] = '';
      }
      $headers[2][] = $value;
      $count++;
    }
  }
  else {
    $headers[0][] = '';
    $headers[1][] = '';
    $headers[2][] = $component['name'];
  }
  return $headers;
}

/**
 * Format the submitted data of a component for CSV downloading.
 */
function _webform_csv_data_product_list($component, $export_options, $value) {

  $data = array();

  switch ($value[0]) {
    case 'checkboxes_product_list':
      unset($value[0]);
      // Separate format
      if ($export_options['select_format'] == 'separate') {
        $count = 0;
        foreach ($component['extra']['products'] as $product) {
          $data[] = in_array($product, $value) ? 'X' : '';
          $count++;
        }
      }
      // Compact format
      else {
        $data_string;
        foreach ($value as $product) {
          $node = node_load($product);
          $data_string .= $node->model . ",";
        }
        $data[] = rtrim($data_string, ',');
      }
      break;
    case 'radio_product_list';
      $node = node_load($value[1]);
      $data[] = $node->model;
      break;
  }

  return $data;
}

function _uc_webform_products($component) {

  $options = array();
  $count = 0;

  foreach ($component['extra']['products'] as $product) {
    $node = node_load($product);
    $options["$node->nid"] = "$node->model";
    $count++;
  }

  return $options;
}