/**
 * @file
 * Field widget behavior of the "icon_icon" widget.
 */
(function ($) {

  "use strict";

  Backdrop.behaviors.iconfieldDialogInsert = {
    attach: function () {
      // Park info to use with 'iconfield:dialogsave'.
      $('a.iconbrowser-opener').once('iconbrowser-opener').on('click', function (event) {
        Backdrop.settings.iconfieldInsertTarget = $(this).data('insert-id');
      });

      // "Remove" button functionality.
      $('button.icon-remove').once('icon-remove').on('click', function (event) {
        const $parent = $(this).closest('.iconfield-wrapper');
        $parent.find('.iconbrowser-target').val('');
        $parent.find('.iconfield-icon-key').html('');
        $parent.find('.iconfield-preview').html('');
        $parent.find('.alt-text').hide();
        $parent.find('.alt-text input').val('');
        $(this).hide();
      });

      // Remove button and alt input initial visibility.
      $('.iconbrowser-target').once('iconbrowser-target').each(function () {
        const $parent = $(this).closest('.iconfield-wrapper');
        if ($(this).val().length) {
          $parent.find('button.icon-remove').show();
          $parent.find('.alt-text').show();
        }
        else {
          $parent.find('button.icon-remove').hide();
          $parent.find('.alt-text').hide();
        }
      });
    }
  };

  $(window).on('iconfield:dialogsave', function (event, data) {
    if (Backdrop.settings.iconfieldInsertTarget !== undefined) {
      const $targetInput = $('#' + Backdrop.settings.iconfieldInsertTarget);
      const $parent = $targetInput.closest('.iconfield-wrapper');
      $targetInput.val(data.project_icon_key);
      $parent.find('.update-item-button').trigger('click');
      $parent.find('.alt-text input').val('');
      $parent.find('button.icon-remove').show();
      $parent.find('.alt-text').show();
    }
  });

})(jQuery);
