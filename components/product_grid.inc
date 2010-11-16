<?php
// $Id$

/**
 * @file
 * Webform module grid component.
 */

// Product Grid depends on functions provided by select.
webform_component_include('select');

/**
 * Implementation of _webform_theme_component().
 */
function _webform_theme_product_grid() {
  return array(
    'uc_webform_render_product_grid' => array(
      'arguments' => array('element' => NULL),
    ),
    'uc_webform_display_product_grid' => array(
      'arguments' => array('element' => NULL),
    ),
  );
}

function theme_uc_webform_render_product_grid($element) {
  $temp_rows = array();
  $columns = array(array('data' => '', 'class' => 'uc_webform-product-grid-product'));

  $option_ids = array();
  $product_keys = element_children($element);


  // This loop creates the row at the top (which is a list of product options) and 
  // the first column on the left (which is the product title column).
  foreach (element_children($element) as $key) {
    $product_element = $element[$key];

    // Create a row with the Product title.
    $row = array(array('data' => $product_element['#title'], 'class' => 'uc_webform-product-grid-product', 'product' => "$key"));

    foreach ($product_element['#options'] as $optionid => $opt_val) {
      if (!in_array($optionid, $option_ids)) {
        $columns[] = array('data' => $opt_val, 'class' => 'checkbox uc_webform-product-grid-option', 'oid' => $optionid);
      }
      $option_ids[] = $optionid;
    }
    $temp_rows[] = $row;
  }

  $final_rows = array();
  foreach ($temp_rows as $row) {
    $product_key = $row[0]['product'];
    $radios = expand_radios($element[$product_key]);
    foreach ($columns as $col) {
      if (isset($radios[$col['oid']])) {
        unset($radios[$col['oid']]['#title']);
        $row[] = array('data' => drupal_render($radios[$col['oid']]), 'class' => 'checkbox webform-grid-option');
      }
      elseif (isset($col['oid'])) {
        $row[] = array('data' => 'n/a', 'class' => 'checkbox webform-grid-option');
      }
    }
    $final_rows[] = $row;
  }

  $option_count = count($header) - 1;

  return theme('form_element', $element, theme('table', $columns, $final_rows, array('class' => 'uc_webform-product-grid uc_webform-product-grid-' . $option_count)));
}

function theme_uc_webform_display_product_grid($element) {

  if (isset($element['#value'])) {
    $output = (count($element['#value']) > 1) ? theme('item_list', $element['#value']) : $element['#value'][0];
  }

  return $output;

}

/**
 * Implementation of _webform_defaults_component().
 */
function _webform_defaults_product_grid() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'mandatory' => 0,
    'pid' => 0,
    'weight' => 0,
    'extra' => array(
      'options' => '',
      'questions' => '',
      'optrand' => 0,
      'qrand' => 0,
      'title_display' => 0,
      'custom_option_keys' => 0,
      'custom_question_keys' => 0,
      'description' => '',
    ),
  );
}

/**
 * Implementation of _webform_edit_component().
 */
function _webform_edit_product_grid($component) {
  $form = array();
  $product_types = uc_product_types();

  // I need to limit the users selection to only those products that *do* contain attributes.
  $results = db_query("SELECT n.nid, n.title, p.model FROM {node} AS n INNER JOIN {uc_products} AS p ON n.nid = p.nid WHERE n.nid IN (SELECT nid FROM {uc_product_attributes}) ORDER BY n.title");

  $products = array();
  while ($product = db_fetch_array($results)) {
    $products[$product['nid'] . '_' . $product['model']] = check_plain($product['title']);
  }

  // Most options are stored in the "extra" array, which stores any settings unique to a particular component type.
  $form['extra']['products'] = array(
    '#type' => 'select',
    '#title' => t('Products'),
    '#default_value' => $component['extra']['products'],
    '#multiple' => TRUE,
    '#description' => t('Please select your products. Only products with attributes are displayed.'),
    '#weight' => -3,
    '#size' => 20,
    '#required' => TRUE,
    '#options' => $products,
  );

  return $form;
}

/**
 * Implementation of _webform_render_component().
 */
function _webform_render_product_grid($component) {

  $product_nodes = array();
  $stock_description = "";

  foreach ($component['extra']['products'] as $val) {
    $product_info = explode('_', $val, 2);
    $stock_level = uc_stock_level($product_info[1]);

    // Check stock levels. The product is only selectable if it is in stock.
    if (($stock_level === FALSE) or (intval($stock_level) > 0)) {
      $product_nodes[$val] = node_load($product_info[0]);
    }
    else {
      $node = node_load($product_info[0]);
      $stock_description .= check_plain($node->title) . ' ' . t('is out of stock.') . '<br />';
    }
  }

  $element = array(
    '#title' => $component['name'],
    '#title_display' => $component['extra']['title_display'],
    '#required' => $component['mandatory'],
    '#weight' => $component['weight'],
    '#description' => $component['extra']['description'] . $stock_description,
    '#theme' => 'uc_webform_render_product_grid',
    '#theme_wrappers' => array('webform_element_wrapper'),
    '#pre_render' => array('webform_element_title_display'),
    '#post_render' => array('webform_element_wrapper'),
    '#webform_component' => $component,
  );

  // Iterate through each product that will be offered. Products will appear on the left of the grid.
  foreach ($product_nodes as $key => $product_node) {
    $element[$key] = array(
      '#title' => $product_node->title,
      '#required' => $component['mandatory'],
      '#type' => 'radios',
      '#process' => array('expand_radios', 'webform_expand_select_ids'),
    );

    // Iterate through each product option for each product. Product options will appear on the top of the grid.
    $product_options = array();
    foreach ($product_node->attributes as $aid => $attribute) {
      foreach ($attribute->options as $option) {
        // aid_oid
        $product_options[$aid . '_' . $option->oid] = $option->name;
      }
    }
    $element[$key]['#options'] = $product_options;
  }

  return $element;
}

/**
 * Implementation of _webform_display_component().
 */
function _webform_display_product_grid($component, $value, $format = 'html') {

  $products = array();
  foreach ($value as $key => $val) {
    $nid_sku = explode('_', $key, 2);
    $aid_oid = explode('_', $val, 2);
    $node = node_load($nid_sku[0]);
    $products[] = $node->title . ', SKU: ' . $node->model . ', Attribute: ' . $node->attributes[$aid_oid[0]]->name . ', Option: ' . $node->attributes[$aid_oid[0]]->options[$aid_oid[1]]->name;
  }

  $element = array(
    '#title' => $component['name'],
    '#weight' => $component['weight'],
    '#theme' => 'uc_webform_display_product_grid',
    '#webform_component' => $component,
    '#format' => $format,
    '#value' => $products,
  );

  return $element;
}

/**
 * Implementation of _webform_analysis_component().
 */
function _webform_analysis_product_grid($component, $sids = array()) {

  // Create an entire table to be put into the returned row.
  $rows = array('');
  $header = array('');

  $columns = array(array('data' => '', 'class' => 'uc_webform-product-grid-product'));
  $product_nids = array();

  $option_ids = array();

  foreach ($component['extra']['products'] as $product) {
    $nid_sku = explode('_', $product, 2);
    $product_nids[] = $nid_sku[0];
    $product_node = node_load($nid_sku[0]);

    // I nid_sku combination here as a key
    $rows[$product] = array($product_node->title);
  }

  // Select the available option IDs and option names for each product in the table.
  $result = db_query("SELECT DISTINCT po.oid, ao.name FROM {uc_product_options} AS po INNER JOIN {uc_attribute_options} AS ao ON po.oid = ao.oid WHERE po.nid IN ('". implode("','", $product_nids) ."')");

  // Build the header row
  $options = array();
  while ($option = db_fetch_array($result)) {
    // This is tricky, but I'm using the oid as an array key to help when I
    // query the webforom_submitted_data table (since it stores the oid instead of the 
    // option name).
    $header[$option['oid']] = $option['name'];
  }

  $result = db_query("SELECT no, data, count(data) as datacount FROM {webform_submitted_data} WHERE nid = %d AND cid = %d GROUP BY no, data", $component['nid'], $component['cid']);

  $selections = array();
  while ($data = db_fetch_array($result)) {
    $aid_oid = explode('_', $data['data'], 2);
    // The information is stored in the DB as attribute-ID_option-ID. This line
    // removes the attribute ID and leaves the $selections key as the option ID.
    $selections[$data['no']][$aid_oid[1]] = $data['datacount'];
  }

  foreach ($rows as $rkey => $rval) {
    if ($rkey != 0) {
      foreach ($header as $hkey => $hval) {
        if ($hkey != 0) {
          // $rkey = product-nid_sku
          // $hkey = option ID
          $rows[$rkey][] = isset($selections[$rkey][$hkey]) ? $selections[$rkey][$hkey] : 0;
        }
      }
    }
  }

  $output = theme('table', $header, $rows, array('class' => 'webform-product-grid'));

  return array(array(array('data' => $output, 'colspan' => 2)));
}

/**
 * Implementation of _webform_table_component().
 */
function _webform_table_product_grid($component, $value) {
  
  $output = '';

  // Set the value as a single string.
  foreach ($value as $key => $val) {
    $nid_sku = explode('_', $key, 2);
    $aid_oid = explode('_', $val, 2);

    $node = node_load($nid_sku[0]);
    // Output the SKU and the option
    $output .= $node->model . ' - ' . $node->attributes[$aid_oid[0]]->options[$aid_oid[1]]->name . '<br />';
  }

  return $output;
}

/**
 * Implementation of _webform_csv_headers_component().
 */
function _webform_csv_headers_product_grid($component, $export_options) {
  $headers = array(
    0 => array(),
    1 => array(),
    2 => array(),
  );

  if ($export_options['select_format'] == 'separate') {
    $headers[0][] = '';
    $headers[1][] = $component['name'];

    $count = 0;
    foreach ($component['extra']['products'] as $val) {
      $product_info = explode('_', $val, 2);
      $node = node_load($product_info[0]);
        // Add blank cells
        if ($count != 0) {
          // Empty column per sub-field in main header.
          $headers[0][] = '';
          $headers[1][] = '';
        }

        // List the product options as header cells
        $headers[2][] = $node->title;
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
 * Implementation of _webform_csv_data_component().
 */
function _webform_csv_data_product_grid($component, $export_options, $value) {

  $data = array();

  // Separate format
  if ($export_options['select_format'] == 'separate') {
    foreach ($component['extra']['products'] as $product) {
      if (isset($value[$product])) {
        $nid_sku = explode('_', $product, 2);
        $aid_oid = explode('_', $value[$product], 2);

        $node = node_load($nid_sku[0]);

        $data[] = $node->attributes[$aid_oid[0]]->options[$aid_oid[1]]->name;
      }
    }
  }
  // Compact format
  else {
    $data_string;
    foreach ($component['extra']['products'] as $product) {
      if (isset($value[$product])) {
        $nid_sku = explode('_', $product, 2);
        $aid_oid = explode('_', $value[$product], 2);

        $node = node_load($nid_sku[0]);

        $data_string .= $node->attributes[$aid_oid[0]]->options[$aid_oid[1]]->name . ',';
      }
    }
    $data[] = rtrim($data_string, ',');
  }

  return $data;
}