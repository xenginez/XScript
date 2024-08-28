#include <list>
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
	enum class client_status
	{
		READ,
		WRITE,
		CLOSE,
	};

	struct json_parse
	{
		template<typename Iterator> bool operator()( Iterator beg, Iterator end )
		{
			x::uint64 layer = 0;
			while ( beg != end )
			{
				if ( *beg == '{' || *beg == '[' )
					layer++;
				else if ( *beg == '}' || *beg == ']' )
					layer--;

				++beg;

				if ( layer == 0 )
					break;
			}

			return ( layer == 0 );
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
		x::symbols symbols = {};
		document_map documents = {};
	};
	using workspace_map = std::map<std::string, workspace>;

	struct client : public std::enable_shared_from_this<client>
	{
		x_socket socket;
		json_parse parse;
		x::coroutine coro = {};
		client_status status = client_status::READ;
		x::buffer rbuf = {};
		x::buffer wbuf = {};
		workspace_map workspaces;
	};
	using client_ptr = std::shared_ptr<client>;
	using client_map = std::map<x_socket, client_ptr>;

}

class xlserver
{
private:
	using method_type = x::json( xlserver:: * )( const x::json & );

private:
	x::grammar_ptr _grammar;

	client_map _clients;
	x_socket _socket = nullptr;
	x::coroutine _coaccept = {};
	std::list<client_ptr> _closes = {};
	std::map<std::string, method_type> _methods;

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
	int run( x::uint16 port = 18080 )
	{
		_grammar = std::make_shared<x::grammar>();

		if ( _socket = x_socket_create( SOCKET_PROTOCOL_TCP, SOCKET_AF_INET ) )
		{
			x_socket_bind( _socket, "127.0.0.1", port );
			x_socket_listen( _socket );
			x_coroutine_socket_accept( &_coaccept, _socket );
		}

		while ( 1 )
		{
			while ( !_closes.empty() )
			{
				auto client = _closes.front();

				auto it = _clients.find( client->socket );
				if ( it != _clients.end() )
					_clients.erase( it );

				x_socket_close( client->socket );
				x_socket_release( client->socket );

				_closes.pop_front();
			}

			if ( _coaccept.next() )
			{
				auto socket = _coaccept.value().to_intptr();
				_coaccept.reset();

				if ( auto c = std::make_shared<client>() )
				{
					c->socket = socket;
					_clients.insert( { c->socket, c } );

					x_coroutine_socket_recv( &c->coro, c->socket, c->rbuf.prepare( 512 ), 512 );
				}

				x_coroutine_socket_accept( &_coaccept, _socket );
			}
			else if ( _coaccept.error() )
			{
				std::cout << _coaccept.exception().what() << std::endl;
			}

			for ( auto & it : _clients )
			{
				auto client = it.second;

				if ( client->coro.next() )
				{
					if ( client->status == client_status::READ )
					{
						client->rbuf.commit( client->coro.value().to_uint64() );
						client->coro.reset();

						if ( client->parse( client->rbuf.begin(), client->rbuf.end() ) )
						{
							auto request = x::json::load( &client->rbuf );

							auto response = dispatch( request, client );

							if ( !response.empty() )
							{
								response.save( &client->wbuf, true );

								std::cout << "request: " << request << "\n" << "response: " << response << std::endl;

								client->status = client_status::WRITE;
								x_coroutine_socket_send( &client->coro, client->socket, (intptr)( client->wbuf.data() ), client->wbuf.size() );
							}
							else
							{
								client->status = client_status::READ;
								x_coroutine_socket_recv( &client->coro, client->socket, client->rbuf.prepare( 512 ), 512 );
							}
						}
					}
					else if( client->status == client_status::WRITE )
					{
						client->wbuf.consume( client->coro.value().to_uint64() );
						client->coro.reset();

						if ( client->wbuf.size() != 0 )
						{
							client->status = client_status::WRITE;
							x_coroutine_socket_send( &client->coro, client->socket, (intptr)( client->wbuf.data() ), client->wbuf.size() );
						}
						else
						{
							client->status = client_status::READ;
							x_coroutine_socket_recv( &client->coro, client->socket, client->rbuf.prepare( 512 ), 512 );
						}
					}
				}
				else if ( client->coro.error() )
				{
					std::cout << client->coro.exception().what() << std::endl;

					client->status = client_status::CLOSE;

					_closes.push_back( client );
				}
			}
		}

		x_socket_close( _socket );
		x_socket_release( _socket );

		return 0;
	}

private:
	x::json dispatch( const x::json & request, const client_ptr & client )
	{
		x::json response;

		if ( request.contains( "method" ) )
		{
			auto it = _methods.find( request["method"] );
			if ( it != _methods.end() )
			{
				response = ( this->*it->second )( request["params"] );
			}
		}
		else // err
		{
			response["code"] = -1;
			response["message"] = "";
		}

		return response;
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
