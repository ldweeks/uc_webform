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
  dd($element, "Product Grid Element in Theme Function");
  $rows = array();
  $header = array(array('data' => '', 'class' => 'uc_webform-product-grid-product'));
  $first = TRUE;
  /*foreach (element_children($element) as $key) {
    $product_element = $element[$key];

    // Set the header for the table.
    if ($first) {
      foreach ($product_element['#options'] as $option) {
        $header[] = array('data' => _webform_filter_xss($option), 'class' => 'checkbox webform-grid-option');
      }
      $first = FALSE;
    }

    // Create a row with the question title.
    $row = array(array('data' => _webform_filter_xss($question_element['#title']), 'class' => 'webform-grid-question'));

    // Render each radio button in the row.
    $radios = expand_radios($question_element);
    foreach (element_children($radios) as $key) {
      unset($radios[$key]['#title']);
      $row[] = array('data' => drupal_render($radios[$key]), 'class' => 'checkbox webform-grid-option');
    }
    $rows[] = $row;
  }*/

  $option_count = count($header) - 1;
  return theme('form_element', $element, theme('table', $header, $rows, array('class' => 'uc_webform-product-grid uc_webform-product-grid-' . $option_count)));
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

  dpm($product_nodes, "Product Grid Nodes");

  $element = array(
    '#title' => $component['name'],
    '#title_display' => $component['extra']['title_display'],
    '#required' => $component['mandatory'],
    '#weight' => $component['weight'],
    '#description' => ($component['extra']['description']),
    '#theme' => 'uc_webform_product_grid',
    '#theme_wrappers' => array('webform_element_wrapper'),
    '#pre_render' => array('webform_element_title_display'),
    '#post_render' => array('webform_element_wrapper'),
    '#webform_component' => $component,
  );

  $products = array();
  foreach ($product_nodes as $key => $product_node) {
    // These products will appear in the header of the grid.
    $products[$key] = $product_node->title;
  }

  $element[$key]['#options'] = $products;
//---------------------NOW IN NEED TO FIGURE OUT HOW TO MATCH THE product options down the left for each product across the top.

    // These "product_options" are the actual *product options* for a particular product.
    // They will appear on the left of a grid.
    $product_options = $product_node->attributes[1]->options;
    foreach ($product_options as $product_option) {
      $options[$option->oid] = $option->name;
    }

  foreach ($product_options as $option) {
    $options[$option->oid] = $option->name;
  }

  foreach ($product_nodes as $key => $product_node) {
    $element[$key] = array(
      '#title' => $product_node->title,
      '#required' => $component['mandatory'],
      '#type' => 'radios',
      '#process' => array('expand_radios', 'webform_expand_select_ids'),
    );
  }

  dd($element, "Product Grid Element as returned from Render");

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