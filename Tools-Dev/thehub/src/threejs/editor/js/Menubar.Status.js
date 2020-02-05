/**
 * @author mrdoob / http://mrdoob.com/
 */

Menubar.Status = function ( editor ) {

	var strings = editor.strings;

	var container = new UI.Panel();
	container.setClass( 'menu right' );

	var autosave = new UI.THREE.Boolean( editor.config.getKey( 'autosave' ), strings.getKey( 'menubar/status/autosave' ) );
	autosave.text.setColor( '#888' );
	autosave.onChange( function () {

		var value = this.getValue();

		editor.config.setKey( 'autosave', value );

		if ( value === true ) {

			editor.signals.sceneGraphChanged.dispatch();

		}

	} );
	container.add( autosave );
	var status = new UI.Text( "" );
	status.setClass( 'title' );
	status.setOpacity( 0.5 );
	container.add( status );

	editor.signals.savingStarted.add( function () {

		autosave.text.setTextDecoration( 'underline' );
		editor.signals.assetLoad.dispatch();
	} );

	editor.signals.savingFinished.add( function () {
		autosave.text.setTextDecoration( 'none' );
		editor.signals.assetLoaded.dispatch();
	} );

	editor.signals.assetLoad.add( function () {
		status.setInnerHtml('<img style="-webkit-user-select: none;" src="./images/loading.gif">');
	} );

	editor.signals.assetLoaded.add( function () {
		status.setTextContent("");
	} );

	editor.signals.objectAdded.add( function () {
		status.setTextContent("Status: loaded")
	} );

	var version = new UI.Text( 'r' + THREE.REVISION );
	version.setClass( 'title' );
	version.setOpacity( 0.5 );
	container.add( version );

	return container;

};
