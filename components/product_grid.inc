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
    'uc_webform_product_grid' => array(
      'arguments' => array('grid_element' => NULL),
    ),
    //'webform_display_grid' => array(
    //  'arguments' => array('element' => NULL),
    //),
  );
}

function theme_uc_webform_product_grid($element) {
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
    '#theme' => 'uc_webform_product_grid',
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
 *
function _webform_display_grid($component, $value, $format = 'html') {
  $questions = _webform_select_options_from_text($component['extra']['questions'], TRUE);
  $options = _webform_select_options_from_text($component['extra']['options'], TRUE);

  $element = array(
    '#title' => $component['name'],
    '#weight' => $component['weight'],
    '#format' => $format,
    '#questions' => $questions,
    '#options' => $options,
    '#theme' => 'webform_display_grid',
    '#theme_wrappers' => $format == 'html' ? array('webform_element', 'webform_element_wrapper') : array('webform_element_text'),
    '#post_render' => array('webform_element_wrapper'),
    '#sorted' => TRUE,
    '#webform_component' => $component,
  );

  foreach ($questions as $key => $question) {
    if ($question !== '') {
      $element[$question] = array(
        '#title' => $question,
        '#value' => isset($value[$key]) ? $value[$key] : NULL,
      );
    }
  }

  return $element;
}

/**
 * Format the text output for this component.
 *
function theme_webform_display_grid($element) {
  $component = $element['#webform_component'];
  $format = $element['#format'];

  if ($format == 'html') {
    $rows = array();
    $header = array(array('data' => '', 'class' => 'webform-grid-question'));
    foreach ($element['#options'] as $option) {
      $header[] = array('data' => _webform_filter_xss($option), 'class' => 'checkbox webform-grid-option');
    }
    foreach (element_children($element) as $key) {
      $row = array();
      $row[] = array('data' => _webform_filter_xss($element[$key]['#title']), 'class' => 'webform-grid-question');
      foreach ($element['#options'] as $option_value => $option_label) {
        if (strcmp($option_value, $element[$key]['#value']) == 0) {
          $row[] = array('data' => '<strong>X</strong>', 'class' => 'checkbox webform-grid-option');
        }
        else {
          $row[] = array('data' => '&nbsp', 'class' => 'checkbox webform-grid-option');
        }
      }
      $rows[] = $row;
    }

    $option_count = count($header) - 1;
    $output = theme('table', $header, $rows, array('class' => 'webform-grid webform-grid-' . $option_count));
  }
  else {
    $items = array();
    foreach (element_children($element) as $key) {
      $items[] = ' - ' . $element[$key]['#title'] . ': ' . (isset($element['#options'][$element[$key]['#value']]) ? $element['#options'][$element[$key]['#value']] : '');
    }
    $output = implode("\n", $items);
  }

  return $output;
}

/**
 * Implementation of _webform_analysis_component().
 *
function _webform_analysis_grid($component, $sids = array()) {
  // Generate the list of options and questions.
  $options = _webform_select_options_from_text($component['extra']['options'], TRUE);
  $questions = _webform_select_options_from_text($component['extra']['questions'], TRUE);

  // Generate a lookup table of results.
  $placeholders = count($sids) ? array_fill(0, count($sids), "'%s'") : array();
  $sidfilter = count($sids) ? " AND sid in (" . implode(",", $placeholders) . ")" : "";
  $query = 'SELECT no, data, count(data) as datacount ' .
    ' FROM {webform_submitted_data} ' .
    ' WHERE nid = %d ' .
    ' AND cid = %d ' .
    " AND data != '' " . $sidfilter .
    ' GROUP BY no, data';
  $result = db_query($query, array_merge(array($component['nid'], $component['cid']), $sids));
  $counts = array();
  while ($data = db_fetch_object($result)) {
    $counts[$data->no][$data->data] = $data->datacount;
  }

  // Create an entire table to be put into the returned row.
  $rows = array();
  $header = array('');

  // Add options as a header row.
  foreach ($options as $option) {
    $header[] = _webform_filter_xss($option);
  }

  // Add questions as each row.
  foreach ($questions as $qkey => $question) {
    $row = array(_webform_filter_xss($question));
    foreach ($options as $okey => $option) {
      $row[] = !empty($counts[$qkey][$okey]) ? $counts[$qkey][$okey] : 0;
    }
    $rows[] = $row;
  }
  $output = theme('table', $header, $rows, array('class' => 'webform-grid'));


  return array(array(array('data' => $output, 'colspan' => 2)));
}

/**
 * Implementation of _webform_table_component().
 *
function _webform_table_grid($component, $value) {
  $questions = _webform_select_options_from_text($component['extra']['questions'], TRUE);
  $options = _webform_select_options_from_text($component['extra']['options'], TRUE);

  $output = '';
  // Set the value as a single string.
  foreach ($questions as $key => $label) {
    if (isset($value[$key]) && isset($options[$value[$key]])) {
      $output .= _webform_filter_xss($label) . ': ' . _webform_filter_xss($options[$value[$key]]) . '<br />';
    }
  }

  return $output;
}

/**
 * Implementation of _webform_csv_headers_component().
 *
function _webform_csv_headers_grid($component, $export_options) {
  $header = array();
  $header[0] = array('');
  $header[1] = array($component['name']);
  $items = _webform_select_options_from_text($component['extra']['questions'], TRUE);
  $count = 0;
  foreach ($items as $key => $item) {
    // Empty column per sub-field in main header.
    if ($count != 0) {
      $header[0][] = '';
      $header[1][] = '';
    }
    // The value for this option.
    $header[2][] = $item;
    $count++;
  }

  return $header;
}

/**
 * Implementation of _webform_csv_data_component().
 *
function _webform_csv_data_grid($component, $export_options, $value) {
  $questions = _webform_select_options_from_text($component['extra']['questions'], TRUE);
  $options = _webform_select_options_from_text($component['extra']['options'], TRUE);
  $return = array();
  foreach ($questions as $key => $question) {
    if (isset($value[$key]) && isset($options[$value[$key]])) {
      $return[] = $export_options['select_keys'] ? $value[$key] : $options[$value[$key]];
    }
  }
  return $return;
}*/