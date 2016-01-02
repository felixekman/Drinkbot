var model = (function(base) {
	
	var db = $.jStorage.get("drinkDatabase_storage");
	var json = $.jStorage.get("data");
	
	
	return {
	
		getJson: function () {
			return  $.jStorage.get("data");
		},
	
		setJson: function () {
			$.ajax({
				url : db,
				dataType:"jsonp",
				jsonp:"mycallback",
				success:function(data, textStatus, jqXHR) {
					$.jStorage.set("data", data);
					$.mobile.changePage( "#home", { allowSamePageTransition: true } );
				},
				error: function(jqXHR, textStatus, errorThrown) {
					window.plugins.toast.showShortCenter("Error loading list.");
				},
				complete: function (jqXHR, textStatus){

				}
			});
		},
		
		deleteJson: function () {
			$.jStorage.set("data", "");
		}
	}
	
}( model || {}));