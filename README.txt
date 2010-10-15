Description
------------
This module integrates two other modules: webform and ubercart. It allows you sell products directly from your webforms. It is particularly suited to be used as part of a registration system.

Requirements
------------
- Drupal 6.x
- Ubercart 2.4
- Webform 3.2
- Unfortunately, it also requires the following patch to Ubercart: http://drupal.org/node/744956#comment-2731836

Installation
-------------
1. Copy the entire uc_webform directory the Drupal sites/all/modules directory.
2. Login as an administrator. Enable the module in the "Administer" -> "Site
   Building" -> "Modules"

Notes
-----
- For this module to be of any value, you must have already created some products on your site.
- After you install this module, you will have two more component types for a webform: 'product' and 'product list'. When you add either type of component, you will be asked to select which products you'd like to offer to those filling out the webform. The products you select will be added to your webform.
- When a user selects a product in a webform, the product is automatically added to their cart when the form is submitted.
- If you add a 'product' or 'product list' component to your webform, a hidden 'Order Status' field will be added to your webform. The values in this field will correspond to the Ubercart order status of the order that was created by the webform submission. You cannot remove an 'Order Status' component from a webform until you have removed all product components from the webform first.
- The Analysis tab (which is available when viewing results for a particular webform) returns product data for *all* webform submissions. That means that clicking on 'Analysis' will display *all* the submission data for a  particular webform regardless of whether or not the user completed the checkout process.
- A new tab called "Orders" appears when you click on the results for a particular webform. This new page only contains from the 'product' or 'product list' components of the given webform, and it only returns data for webform submissions that ended in a completed checkout. This way, you can quickly and easily see how many of a given product was purchased with your webform.
- I do not currently plan to support attributes and options. If that is a need, please submit a patch.

Developer Notes
---------------
- The structure of the 'no' and 'data' columns in the {webform_submitted_data} table is very important. If the component is a 'product' or a 'product list', the value in the data column where 'no = 0' specifies the type of product data. The options are 'product' (user specifies the quantity they'd like in a textbox), 'checkboxes_product_list' and 'radio_product_list'. If the data refers to 'radio_product_list' and 'checkboxes_product_list', the values corresponding to the 'no' column greater than 0 refer to product node id's. If the data refers to a 'product', then 'no' at 1 is the product node id, and the 'no' at 2 is the product quantity.

Todo/Bugs/Feature Requests
--------------------------
- Remove dependency on Ubercart patch.
- Add better support for the analysis tab.

Support
-------
Please use the issue queue for filing bugs with this module at http://drupal.org/project/issues/uc_webform

Credits
-------
Authored and maintained by Lucas Weeks (ldweeks).
Sponsored by Atrium Web Services - http://atriumwebservices.com