/**
 * Allow to request inserting into tags by browser link data attribute.
 * @todo over-abstraction or good idea?
 */
(function ($) {

  "use strict";

  Backdrop.behaviors.iconfieldDialogAddSettings = {
    attach: function (context, settings) {
      $('[data-insert-options]').on('click', function (event) {
        Backdrop.settings.iconfieldInsert = $(this).data('insert-options');
      });
    }
  };

  // @see Backdrop.ajax.prototype.commands.iconFieldDialogSave
  $(window).on('iconfield:dialogsave', function (event, data) {
    if (Backdrop.settings.iconfieldInsert !== undefined) {
      let options = Backdrop.settings.iconfieldInsert;
      if (options.insertType && options.targetSelector) {
        // form item or not - val() vs. html().
        let $targetElement = $(options.targetSelector);
        if ($targetElement[0].form !== undefined) {
          $targetElement.val(data[options.insertType]);
        }
        else {
          $targetElement.html(data[options.insertType]);
        }
      }
    }
  });

})(jQuery);
