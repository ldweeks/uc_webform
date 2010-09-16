Description
------------
This module integrates two other modules: webform and ubercart. It allows you sell products directly from your webforms. It is particularly suited to be used as a registration system.

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

Todo/Bugs/Feature Requests
--------------------------
- Remove dependency on Ubercart patch.
- Include support for attributes and options.
- Include support for stock levels.
- Add better support for the analysis tab.

Support
-------
Please use the issue queue for filing bugs with this module at http://drupal.org/project/issues/uc_webform

Credits
-------
Authored and maintained by Lucas Weeks (ldweeks).
Sponsored by Atrium Web Services - http://atriumwebservices.com