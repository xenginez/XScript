#include <functional>

#include <xlib.h>
#include <json.hpp>
#include <grammar.h>
#include <symbols.h>
#include <concurrent_queue.hpp>

namespace
{
	struct document
	{
		std::string text;
	};
	using document_map = std::map<std::string, document>;

	struct workspace
	{
		x::symbols symbols;
		document_map documents;
	};
	using workspace_map = std::map<std::string, workspace>;
}

class xlserver
{
private:
	x_socket _socket = nullptr;
	x::grammar_ptr _grammar;
	std::map<std::string, workspace_map> _clients;

	x_condition _condition = nullptr;
	x::concurrent_queue<std::function<void()>> _taskqueue;

public:
	xlserver() = default;
	~xlserver() = default;

public:
	int run( x::uint16 port = 8080 )
	{
		_grammar = std::make_shared<x::grammar>();
		_socket = x_socket_create( SOCKET_PROTOCOL_TCP, SOCKET_AF_INET );
		x_socket_bind( _socket, "127.0.0.1", port );
		x_socket_accept( _socket );

		x_coroutine_socket_accept();
		_condition = x_condition_create();
		std::function<void()> task;
		while ( 1 )
		{
			if ( _taskqueue.try_dequeue( task ) )
			{
				task();
			}
			else
			{
				x_condition_wait( _condition );
			}
		}

		return 0;
	}

	template<typename F> void post( F && task )
	{
		_taskqueue.enqueue( task );
		x_condition_notify_one( _condition );
	}

private:
	x::json textDocument_implementation( const x::json & params );
	x::json textDocument_typeDefinition( const x::json & params );
	x::json workspace_workspaceFolders( const x::json & params );
	x::json workspace_configuration( const x::json & params );
	x::json textDocument_documentColor( const x::json & params );
	x::json textDocument_colorPresentation( const x::json & params );
	x::json textDocument_foldingRange( const x::json & params );
	x::json workspace_foldingRange_refresh( const x::json & params );
	x::json textDocument_declaration( const x::json & params );
	x::json textDocument_selectionRange( const x::json & params );
	x::json window_workDoneProgress_create( const x::json & params );
	x::json textDocument_prepareCallHierarchy( const x::json & params );
	x::json callHierarchy_incomingCalls( const x::json & params );
	x::json callHierarchy_outgoingCalls( const x::json & params );
	x::json textDocument_semanticTokens_full( const x::json & params );
	x::json textDocument_semanticTokens_full_delta( const x::json & params );
	x::json textDocument_semanticTokens_range( const x::json & params );
	x::json workspace_semanticTokens_refresh( const x::json & params );
	x::json window_showDocument( const x::json & params );
	x::json textDocument_linkedEditingRange( const x::json & params );
	x::json workspace_willCreateFiles( const x::json & params );
	x::json workspace_willRenameFiles( const x::json & params );
	x::json workspace_willDeleteFiles( const x::json & params );
	x::json textDocument_moniker( const x::json & params );
	x::json textDocument_prepareTypeHierarchy( const x::json & params );
	x::json typeHierarchy_supertypes( const x::json & params );
	x::json typeHierarchy_subtypes( const x::json & params );
	x::json textDocument_inlineValue( const x::json & params );
	x::json workspace_inlineValue_refresh( const x::json & params );
	x::json textDocument_inlayHint( const x::json & params );
	x::json inlayHint_resolve( const x::json & params );
	x::json workspace_inlayHint_refresh( const x::json & params );
	x::json textDocument_diagnostic( const x::json & params );
	x::json workspace_diagnostic( const x::json & params );
	x::json workspace_diagnostic_refresh( const x::json & params );
	x::json textDocument_inlineCompletion( const x::json & params );
	x::json client_registerCapability( const x::json & params );
	x::json client_unregisterCapability( const x::json & params );
	x::json initialize( const x::json & params );
	x::json shutdown( const x::json & params );
	x::json window_showMessageRequest( const x::json & params );
	x::json textDocument_willSaveWaitUntil( const x::json & params );
	x::json textDocument_completion( const x::json & params );
	x::json completionItem_resolve( const x::json & params );
	x::json textDocument_hover( const x::json & params );
	x::json textDocument_signatureHelp( const x::json & params );
	x::json textDocument_definition( const x::json & params );
	x::json textDocument_references( const x::json & params );
	x::json textDocument_documentHighlight( const x::json & params );
	x::json textDocument_documentSymbol( const x::json & params );
	x::json textDocument_codeAction( const x::json & params );
	x::json codeAction_resolve( const x::json & params );
	x::json workspace_symbol( const x::json & params );
	x::json workspaceSymbol_resolve( const x::json & params );
	x::json textDocument_codeLens( const x::json & params );
	x::json codeLens_resolve( const x::json & params );
	x::json workspace_codeLens_refresh( const x::json & params );
	x::json textDocument_documentLink( const x::json & params );
	x::json documentLink_resolve( const x::json & params );
	x::json textDocument_formatting( const x::json & params );
	x::json textDocument_rangeFormatting( const x::json & params );
	x::json textDocument_rangesFormatting( const x::json & params );
	x::json textDocument_onTypeFormatting( const x::json & params );
	x::json textDocument_rename( const x::json & params );
	x::json textDocument_prepareRename( const x::json & params );
	x::json workspace_executeCommand( const x::json & params );
	x::json workspace_applyEdit( const x::json & params );

public:
	x::json workspace_didChangeWorkspaceFolders( const x::json & params );
	x::json window_workDoneProgress_cancel( const x::json & params );
	x::json workspace_didCreateFiles( const x::json & params );
	x::json workspace_didRenameFiles( const x::json & params );
	x::json workspace_didDeleteFiles( const x::json & params );
	x::json notebookDocument_didOpen( const x::json & params );
	x::json notebookDocument_didChange( const x::json & params );
	x::json notebookDocument_didSave( const x::json & params );
	x::json notebookDocument_didClose( const x::json & params );
	x::json initialized( const x::json & params );
	x::json exit( const x::json & params );
	x::json workspace_didChangeConfiguration( const x::json & params );
	x::json window_showMessage( const x::json & params );
	x::json window_logMessage( const x::json & params );
	x::json telemetry_event( const x::json & params );
	x::json textDocument_didOpen( const x::json & params );
	x::json textDocument_didChange( const x::json & params );
	x::json textDocument_didClose( const x::json & params );
	x::json textDocument_didSave( const x::json & params );
	x::json textDocument_willSave( const x::json & params );
	x::json workspace_didChangeWatchedFiles( const x::json & params );
	x::json textDocument_publishDiagnostics( const x::json & params );
	x::json setTrace( const x::json & params );
	x::json logTrace( const x::json & params );
	x::json cancelRequest( const x::json & params );
	x::json progress( const x::json & params );
};

int main()
{
	xlserver server;

	return server.run();
}
