#include <functional>

#include <xlib.h>
#include <value.h>
#include <buffer.h>
#include <object.h>
#include <json.hpp>
#include <grammar.h>
#include <symbols.h>
#include <concurrent_queue.hpp>

namespace
{
	struct json_parse
	{
		std::pair<bool, x::byte *> operator()( x::byte * beg, x::byte * end )
		{
			x::uint64 layer = 0;
			while ( beg != end )
			{
				if ( (char)*beg == '{' || (char)*beg == '[' )
					layer++;
				else if ( (char)*beg == '}' || (char)*beg == ']' )
					layer--;

				++beg;

				if ( layer == 0 )
					break;
			}

			if ( layer == 0 )
				return { true, beg };

			return { false, nullptr };
		}
	};

	struct document
	{
		std::string text;
	};
	using document_map = std::map<std::string, document>;

	struct workspace
	{
		std::string id = {};
		x::coroutine * co = {};
		x::symbols symbols = {};
		document_map documents = {};
	};
	using workspace_map = std::map<std::string, workspace>;

	struct client
	{
		x_socket socket;
		json_parse parse;
		x::coroutine * co = {};
		x::stream_buffer buf;
		workspace_map workspaces;
	};
	using client_map = std::map<x_socket, client>;

}

class xlserver
{
private:
	using method_type = x::json( xlserver:: * )( const x::json & );

private:
	x::grammar_ptr _grammar;

	client_map _clients;
	x_socket _socket = nullptr;
	x_condition _condition = nullptr;
	x::coroutine * _coaccept = nullptr;
	std::map<std::string, method_type> _methods;
	x::concurrent_queue<std::function<bool()>> _taskqueue;

public:
	xlserver()
	{
		_methods["textDocument/implementation"] = &xlserver::textDocument_implementation;
		_methods["textDocument/typeDefinition"] = &xlserver::textDocument_typeDefinition;
		_methods["workspace/workspaceFolders"] = &xlserver::workspace_workspaceFolders;
		_methods["workspace/configuration"] = &xlserver::workspace_configuration;
		_methods["textDocument/documentColor"] = &xlserver::textDocument_documentColor;
		_methods["textDocument/colorPresentation"] = &xlserver::textDocument_colorPresentation;
		_methods["textDocument/foldingRange"] = &xlserver::textDocument_foldingRange;
		_methods["workspace/foldingRange/refresh"] = &xlserver::workspace_foldingRange_refresh;
		_methods["textDocument/declaration"] = &xlserver::textDocument_declaration;
		_methods["textDocument/selectionRange"] = &xlserver::textDocument_selectionRange;
		_methods["window/workDoneProgress/create"] = &xlserver::window_workDoneProgress_create;
		_methods["textDocument/prepareCallHierarchy"] = &xlserver::textDocument_prepareCallHierarchy;
		_methods["callHierarchy/incomingCalls"] = &xlserver::callHierarchy_incomingCalls;
		_methods["callHierarchy/outgoingCalls"] = &xlserver::callHierarchy_outgoingCalls;
		_methods["textDocument/semanticTokens/full"] = &xlserver::textDocument_semanticTokens_full;
		_methods["textDocument/semanticTokens/full/delta"] = &xlserver::textDocument_semanticTokens_full_delta;
		_methods["textDocument/semanticTokens/range"] = &xlserver::textDocument_semanticTokens_range;
		_methods["workspace/semanticTokens/refresh"] = &xlserver::workspace_semanticTokens_refresh;
		_methods["window/showDocument"] = &xlserver::window_showDocument;
		_methods["textDocument/linkedEditingRange"] = &xlserver::textDocument_linkedEditingRange;
		_methods["workspace/willCreateFiles"] = &xlserver::workspace_willCreateFiles;
		_methods["workspace/willRenameFiles"] = &xlserver::workspace_willRenameFiles;
		_methods["workspace/willDeleteFiles"] = &xlserver::workspace_willDeleteFiles;
		_methods["textDocument/moniker"] = &xlserver::textDocument_moniker;
		_methods["textDocument/prepareTypeHierarchy"] = &xlserver::textDocument_prepareTypeHierarchy;
		_methods["typeHierarchy/supertypes"] = &xlserver::typeHierarchy_supertypes;
		_methods["typeHierarchy/subtypes"] = &xlserver::typeHierarchy_subtypes;
		_methods["textDocument/inlineValue"] = &xlserver::textDocument_inlineValue;
		_methods["workspace/inlineValue/refresh"] = &xlserver::workspace_inlineValue_refresh;
		_methods["textDocument/inlayHint"] = &xlserver::textDocument_inlayHint;
		_methods["inlayHint/resolve"] = &xlserver::inlayHint_resolve;
		_methods["workspace/inlayHint/refresh"] = &xlserver::workspace_inlayHint_refresh;
		_methods["textDocument/diagnostic"] = &xlserver::textDocument_diagnostic;
		_methods["workspace/diagnostic"] = &xlserver::workspace_diagnostic;
		_methods["workspace/diagnostic/refresh"] = &xlserver::workspace_diagnostic_refresh;
		_methods["textDocument/inlineCompletion"] = &xlserver::textDocument_inlineCompletion;
		_methods["client/registerCapability"] = &xlserver::client_registerCapability;
		_methods["client/unregisterCapability"] = &xlserver::client_unregisterCapability;
		_methods["initialize"] = &xlserver::initialize;
		_methods["shutdown"] = &xlserver::shutdown;
		_methods["window/showMessageRequest"] = &xlserver::window_showMessageRequest;
		_methods["textDocument/willSaveWaitUntil"] = &xlserver::textDocument_willSaveWaitUntil;
		_methods["textDocument/completion"] = &xlserver::textDocument_completion;
		_methods["completionItem/resolve"] = &xlserver::completionItem_resolve;
		_methods["textDocument/hover"] = &xlserver::textDocument_hover;
		_methods["textDocument/signatureHelp"] = &xlserver::textDocument_signatureHelp;
		_methods["textDocument/definition"] = &xlserver::textDocument_definition;
		_methods["textDocument/references"] = &xlserver::textDocument_references;
		_methods["textDocument/documentHighlight"] = &xlserver::textDocument_documentHighlight;
		_methods["textDocument/documentSymbol"] = &xlserver::textDocument_documentSymbol;
		_methods["textDocument/codeAction"] = &xlserver::textDocument_codeAction;
		_methods["codeAction/resolve"] = &xlserver::codeAction_resolve;
		_methods["workspace/symbol"] = &xlserver::workspace_symbol;
		_methods["workspaceSymbol/resolve"] = &xlserver::workspaceSymbol_resolve;
		_methods["textDocument/codeLens"] = &xlserver::textDocument_codeLens;
		_methods["codeLens/resolve"] = &xlserver::codeLens_resolve;
		_methods["workspace/codeLens/refresh"] = &xlserver::workspace_codeLens_refresh;
		_methods["textDocument/documentLink"] = &xlserver::textDocument_documentLink;
		_methods["documentLink/resolve"] = &xlserver::documentLink_resolve;
		_methods["textDocument/formatting"] = &xlserver::textDocument_formatting;
		_methods["textDocument/rangeFormatting"] = &xlserver::textDocument_rangeFormatting;
		_methods["textDocument/rangesFormatting"] = &xlserver::textDocument_rangesFormatting;
		_methods["textDocument/onTypeFormatting"] = &xlserver::textDocument_onTypeFormatting;
		_methods["textDocument/rename"] = &xlserver::textDocument_rename;
		_methods["textDocument/prepareRename"] = &xlserver::textDocument_prepareRename;
		_methods["workspace/executeCommand"] = &xlserver::workspace_executeCommand;
		_methods["workspace/applyEdit"] = &xlserver::workspace_applyEdit;
		_methods["workspace/didChangeWorkspaceFolders"] = &xlserver::workspace_didChangeWorkspaceFolders;
		_methods["window/workDoneProgress/cancel"] = &xlserver::window_workDoneProgress_cancel;
		_methods["workspace/didCreateFiles"] = &xlserver::workspace_didCreateFiles;
		_methods["workspace/didRenameFiles"] = &xlserver::workspace_didRenameFiles;
		_methods["workspace/didDeleteFiles"] = &xlserver::workspace_didDeleteFiles;
		_methods["notebookDocument/didOpen"] = &xlserver::notebookDocument_didOpen;
		_methods["notebookDocument/didChange"] = &xlserver::notebookDocument_didChange;
		_methods["notebookDocument/didSave"] = &xlserver::notebookDocument_didSave;
		_methods["notebookDocument/didClose"] = &xlserver::notebookDocument_didClose;
		_methods["initialized"] = &xlserver::initialized;
		_methods["exit"] = &xlserver::exit;
		_methods["workspace/didChangeConfiguration"] = &xlserver::workspace_didChangeConfiguration;
		_methods["window/showMessage"] = &xlserver::window_showMessage;
		_methods["window/logMessage"] = &xlserver::window_logMessage;
		_methods["telemetry/event"] = &xlserver::telemetry_event;
		_methods["textDocument/didOpen"] = &xlserver::textDocument_didOpen;
		_methods["textDocument/didChange"] = &xlserver::textDocument_didChange;
		_methods["textDocument/didClose"] = &xlserver::textDocument_didClose;
		_methods["textDocument/didSave"] = &xlserver::textDocument_didSave;
		_methods["textDocument/willSave"] = &xlserver::textDocument_willSave;
		_methods["workspace/didChangeWatchedFiles"] = &xlserver::workspace_didChangeWatchedFiles;
		_methods["textDocument/publishDiagnostics"] = &xlserver::textDocument_publishDiagnostics;
		_methods["$/setTrace"] = &xlserver::setTrace;
		_methods["$/logTrace"] = &xlserver::logTrace;
		_methods["$/cancelRequest"] = &xlserver::cancelRequest;
		_methods["$/progress"] = &xlserver::progress;
	}
	~xlserver() = default;

public:
	int run( x::uint16 port = 8080 )
	{
		_grammar = std::make_shared<x::grammar>();
		_socket = x_socket_create( SOCKET_PROTOCOL_TCP, SOCKET_AF_INET );
		_condition = x_condition_create();
		_coaccept = new x::coroutine;

		x_socket_bind( _socket, "127.0.0.1", port );
		x_socket_listen( _socket );
		accept();

		std::function<bool()> task;
		while ( 1 )
		{
			if ( _taskqueue.try_dequeue( task ) )
				if ( !task() )
					_taskqueue.enqueue( task );
			else
				x_condition_wait( _condition );
		}

		x_socket_close( _socket );
		x_socket_release( _socket );
		x_condition_release( _condition );

		return 0;
	}
	template<typename F> void post( F && task )
	{
		_taskqueue.enqueue( task );

		x_condition_notify_one( _condition );
	}

private:
	void accept()
	{
		x_coroutine_socket_accept( _coaccept, _socket );

		post( [this]()
		{
			if ( _coaccept->next() )
			{
				client c;

				c.co = new x::coroutine;
				c.socket = _coaccept->value().to_intptr();
				_clients.insert( { c.socket, c } );

				read( _clients[c.socket] );

				_coaccept->clear();

				accept();

				return true;
			}
			else if ( _coaccept->error() )
			{

			}

			return false;
		} );
	}

	void read( client & client )
	{
		x_coroutine_socket_recv( client.co, client.socket, client.buf.prepare( 512 ), 512 );

		post( [this, &client]()
		{
			if ( client.co->next() )
			{
				client.buf.commit( client.co->value().to_uint64() );
				auto result = client.parse( client.buf.data(), client.buf.data() + client.buf.size() );
				if ( result.first )
				{
					x::json request = x::json::load( { (const char *)client.buf.data(), (const char *)result.second } );
					auto respone = dispatch( request, client );
					write( client, respone );
				}

				read( client );

				return true;
			}
			else if ( client.co->error() )
			{

			}

			return false;
		} );
	}

	void write( client & client, const x::json & respone )
	{

	}

	x::json dispatch( const x::json & request, client & client )
	{
		if ( request.contains( "method" ) )
		{
			auto it = _methods.find( request["method"] );
			if ( it != _methods.end() )
			{
				return ( this->*it->second )( request["params"] );
			}
		}
		else // err
		{

		}

		return {};
	}

private:
	x::json textDocument_implementation( const x::json & params )
	{
		return {};
	}
	x::json textDocument_typeDefinition( const x::json & params )
	{
		return {};
	}
	x::json workspace_workspaceFolders( const x::json & params )
	{
		return {};
	}
	x::json workspace_configuration( const x::json & params )
	{
		return {};
	}
	x::json textDocument_documentColor( const x::json & params )
	{
		return {};
	}
	x::json textDocument_colorPresentation( const x::json & params )
	{
		return {};
	}
	x::json textDocument_foldingRange( const x::json & params )
	{
		return {};
	}
	x::json workspace_foldingRange_refresh( const x::json & params )
	{
		return {};
	}
	x::json textDocument_declaration( const x::json & params )
	{
		return {};
	}
	x::json textDocument_selectionRange( const x::json & params )
	{
		return {};
	}
	x::json window_workDoneProgress_create( const x::json & params )
	{
		return {};
	}
	x::json textDocument_prepareCallHierarchy( const x::json & params )
	{
		return {};
	}
	x::json callHierarchy_incomingCalls( const x::json & params )
	{
		return {};
	}
	x::json callHierarchy_outgoingCalls( const x::json & params )
	{
		return {};
	}
	x::json textDocument_semanticTokens_full( const x::json & params )
	{
		return {};
	}
	x::json textDocument_semanticTokens_full_delta( const x::json & params )
	{
		return {};
	}
	x::json textDocument_semanticTokens_range( const x::json & params )
	{
		return {};
	}
	x::json workspace_semanticTokens_refresh( const x::json & params )
	{
		return {};
	}
	x::json window_showDocument( const x::json & params )
	{
		return {};
	}
	x::json textDocument_linkedEditingRange( const x::json & params )
	{
		return {};
	}
	x::json workspace_willCreateFiles( const x::json & params )
	{
		return {};
	}
	x::json workspace_willRenameFiles( const x::json & params )
	{
		return {};
	}
	x::json workspace_willDeleteFiles( const x::json & params )
	{
		return {};
	}
	x::json textDocument_moniker( const x::json & params )
	{
		return {};
	}
	x::json textDocument_prepareTypeHierarchy( const x::json & params )
	{
		return {};
	}
	x::json typeHierarchy_supertypes( const x::json & params )
	{
		return {};
	}
	x::json typeHierarchy_subtypes( const x::json & params )
	{
		return {};
	}
	x::json textDocument_inlineValue( const x::json & params )
	{
		return {};
	}
	x::json workspace_inlineValue_refresh( const x::json & params )
	{
		return {};
	}
	x::json textDocument_inlayHint( const x::json & params )
	{
		return {};
	}
	x::json inlayHint_resolve( const x::json & params )
	{
		return {};
	}
	x::json workspace_inlayHint_refresh( const x::json & params )
	{
		return {};
	}
	x::json textDocument_diagnostic( const x::json & params )
	{
		return {};
	}
	x::json workspace_diagnostic( const x::json & params )
	{
		return {};
	}
	x::json workspace_diagnostic_refresh( const x::json & params )
	{
		return {};
	}
	x::json textDocument_inlineCompletion( const x::json & params )
	{
		return {};
	}
	x::json client_registerCapability( const x::json & params )
	{
		return {};
	}
	x::json client_unregisterCapability( const x::json & params )
	{
		return {};
	}
	x::json initialize( const x::json & params )
	{
		return {};
	}
	x::json shutdown( const x::json & params )
	{
		return {};
	}
	x::json window_showMessageRequest( const x::json & params )
	{
		return {};
	}
	x::json textDocument_willSaveWaitUntil( const x::json & params )
	{
		return {};
	}
	x::json textDocument_completion( const x::json & params )
	{
		return {};
	}
	x::json completionItem_resolve( const x::json & params )
	{
		return {};
	}
	x::json textDocument_hover( const x::json & params )
	{
		return {};
	}
	x::json textDocument_signatureHelp( const x::json & params )
	{
		return {};
	}
	x::json textDocument_definition( const x::json & params )
	{
		return {};
	}
	x::json textDocument_references( const x::json & params )
	{
		return {};
	}
	x::json textDocument_documentHighlight( const x::json & params )
	{
		return {};
	}
	x::json textDocument_documentSymbol( const x::json & params )
	{
		return {};
	}
	x::json textDocument_codeAction( const x::json & params )
	{
		return {};
	}
	x::json codeAction_resolve( const x::json & params )
	{
		return {};
	}
	x::json workspace_symbol( const x::json & params )
	{
		return {};
	}
	x::json workspaceSymbol_resolve( const x::json & params )
	{
		return {};
	}
	x::json textDocument_codeLens( const x::json & params )
	{
		return {};
	}
	x::json codeLens_resolve( const x::json & params )
	{
		return {};
	}
	x::json workspace_codeLens_refresh( const x::json & params )
	{
		return {};
	}
	x::json textDocument_documentLink( const x::json & params )
	{
		return {};
	}
	x::json documentLink_resolve( const x::json & params )
	{
		return {};
	}
	x::json textDocument_formatting( const x::json & params )
	{
		return {};
	}
	x::json textDocument_rangeFormatting( const x::json & params )
	{
		return {};
	}
	x::json textDocument_rangesFormatting( const x::json & params )
	{
		return {};
	}
	x::json textDocument_onTypeFormatting( const x::json & params )
	{
		return {};
	}
	x::json textDocument_rename( const x::json & params )
	{
		return {};
	}
	x::json textDocument_prepareRename( const x::json & params )
	{
		return {};
	}
	x::json workspace_executeCommand( const x::json & params )
	{
		return {};
	}
	x::json workspace_applyEdit( const x::json & params )
	{
		return {};
	}

public:
	x::json workspace_didChangeWorkspaceFolders( const x::json & params )
	{
		return {};
	}
	x::json window_workDoneProgress_cancel( const x::json & params )
	{
		return {};
	}
	x::json workspace_didCreateFiles( const x::json & params )
	{
		return {};
	}
	x::json workspace_didRenameFiles( const x::json & params )
	{
		return {};
	}
	x::json workspace_didDeleteFiles( const x::json & params )
	{
		return {};
	}
	x::json notebookDocument_didOpen( const x::json & params )
	{
		return {};
	}
	x::json notebookDocument_didChange( const x::json & params )
	{
		return {};
	}
	x::json notebookDocument_didSave( const x::json & params )
	{
		return {};
	}
	x::json notebookDocument_didClose( const x::json & params )
	{
		return {};
	}
	x::json initialized( const x::json & params )
	{
		return {};
	}
	x::json exit( const x::json & params )
	{
		return {};
	}
	x::json workspace_didChangeConfiguration( const x::json & params )
	{
		return {};
	}
	x::json window_showMessage( const x::json & params )
	{
		return {};
	}
	x::json window_logMessage( const x::json & params )
	{
		return {};
	}
	x::json telemetry_event( const x::json & params )
	{
		return {};
	}
	x::json textDocument_didOpen( const x::json & params )
	{
		return {};
	}
	x::json textDocument_didChange( const x::json & params )
	{
		return {};
	}
	x::json textDocument_didClose( const x::json & params )
	{
		return {};
	}
	x::json textDocument_didSave( const x::json & params )
	{
		return {};
	}
	x::json textDocument_willSave( const x::json & params )
	{
		return {};
	}
	x::json workspace_didChangeWatchedFiles( const x::json & params )
	{
		return {};
	}
	x::json textDocument_publishDiagnostics( const x::json & params )
	{
		return {};
	}
	x::json setTrace( const x::json & params )
	{
		return {};
	}
	x::json logTrace( const x::json & params )
	{
		return {};
	}
	x::json cancelRequest( const x::json & params )
	{
		return {};
	}
	x::json progress( const x::json & params )
	{
		return {};
	}
};

int main()
{
	xlserver server;

	return server.run();
}
