/**
 * @file
 * Icon browser behavior.
 */
(function ($) {

  "use strict";

  Backdrop.ajax.prototype.commands.iconFieldDialogSave = function (ajax, response, status) {
    $(window).trigger('iconfield:dialogsave', [response.values]);
  };

  Backdrop.behaviors.iconfieldAjax = {
    getSubmitValues: function () {
      let $form = $('.iconfield-dialog form');
      let submit = {
        form_build_id: $form.find('[name="form_build_id"]').val()
      };
      const $form_items = $('.icon-browser-filters-wrapper [name]');
      for (const item of $form_items) {
        if (item.type === 'submit') {
          continue;
        }
        submit[ $(item).attr('name') ] = $(item).val();
      }
      return submit;
    },
    attach: function (context) {
      // A11Y, keyboard nav and aria roles.
      $('.icon-browser-list').once('browser-list-a11y', function () {
        $(this).attr('aria-role', 'listbox');
        $(this).find('[data-icon-name]').each(function () {
          $(this).attr('tabindex', 0);
          $(this).attr('aria-role', 'option');
          $(this).attr('aria-selected', 'false');
        });
      });
      // Original (hidden) form buttons, not the dialog buttons.
      $('form.iconfield-dialog-form .form-actions input').each(function () {
        $(this).attr('tabindex', -1);
      });

      // Handle selection.
      $('.icon-browser-list .icon-wrapper').on('click keydown', function (event) {
        if (event.type === 'keydown' && event.which !== 13) {
          return;
        }
        $('.icon-browser-list .icon-wrapper[aria-selected="true"]').attr('aria-selected', 'false');
        $(this).attr('aria-selected', 'true');
        $('input[name="selected_icon"]').val(this.dataset.iconName);
      });

      // Pager and AJAX.
      $('.icon-browser-pager-wrapper').once('pager-each').find('a').each(function () {
        let params = new URL(this.href).searchParams;
        this.href = Backdrop.settings.iconfield.pagerBaseUrl + '?' + params.toString();
        let options = {
          type: 'POST',
          selector: '.icon-browser-pager-wrapper a',
          progress: { 'type': 'throbber' },
          event: 'click',
          url: this.href,
          prevent: 'click',
          accepts: {
            json: 'application/vnd.backdrop-ajax'
          }
        };
        let backdropAjax = new Backdrop.ajax('iconfield-browser-dialog-form', this, options);
        // Setting POST data has to happen "last minute" to reliably get the
        // most recent data.
        backdropAjax.beforeSerialize = function (element, options) {
          options.data = Backdrop.behaviors.iconfieldAjax.getSubmitValues();
          // As we override this method, we have to run the original in
          // addition. Otherwise there's a serious problem with
          // ajaxPageState.theme.
          Backdrop.ajax.prototype.beforeSerialize.apply(this, arguments);
        };
      });
    }
  };

})(jQuery);
