/**
 * JEEVAN MARG - Open-from-file Protocol Guard
 * Prevents running frontend directly via file:// protocol.
 * Informs the user to start the backend C++ server via run_server.bat.
 */
(function() {
    if (window.location.protocol === 'file:') {
        var rawPath = window.location.pathname || '';
        var page = rawPath.substring(rawPath.lastIndexOf('/') + 1) || 'index.html';
        if (!page.endsWith('.html')) {
            page = 'index.html';
        }
        var targetUrl = 'http://localhost:8080/' + page;

        // Stop browser from continuing to parse/execute remaining scripts & resources
        if (typeof window.stop === 'function') {
            try {
                window.stop();
            } catch (e) {}
        }

        function renderGuard() {
            document.title = 'JEEVAN MARG - Local Server Required';

            var guardContainer = document.createElement('div');
            guardContainer.id = 'jeevan-marg-file-guard';
            guardContainer.setAttribute('style', [
                'position: fixed',
                'top: 0',
                'left: 0',
                'width: 100vw',
                'height: 100vh',
                'background: #090d16',
                'color: #f1f5f9',
                'font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif',
                'display: flex',
                'align-items: center',
                'justify-content: center',
                'padding: 24px',
                'box-sizing: border-box',
                'z-index: 2147483647'
            ].join(';'));

            guardContainer.innerHTML =
                '<div style="max-width: 580px; width: 100%; background: #131c2e; border: 1px solid #233554; border-radius: 16px; padding: 36px 32px; box-shadow: 0 25px 60px rgba(0,0,0,0.6); text-align: center; box-sizing: border-box;">' +
                    '<div style="width: 64px; height: 64px; margin: 0 auto 20px auto; background: rgba(239, 68, 68, 0.15); border: 1px solid rgba(239, 68, 68, 0.3); border-radius: 50%; display: flex; align-items: center; justify-content: center; font-size: 28px;">' +
                        '⚠️' +
                    '</div>' +
                    '<h1 style="font-size: 24px; font-weight: 800; color: #ffffff; margin: 0 0 10px 0; letter-spacing: -0.02em;">Server Required to Run JEEVAN MARG</h1>' +
                    '<p style="font-size: 14px; color: #94a3b8; line-height: 1.6; margin: 0 0 24px 0;">' +
                        'You opened this file directly via the <code style="background: #1e293b; color: #f87171; padding: 2px 6px; border-radius: 4px; font-size: 13px;">file://</code> protocol. ' +
                        'Browsers restrict REST API calls, database synchronization, and CORS requests from local files.' +
                    '</p>' +
                    '<div style="background: #0a0f1d; border: 1px solid #1e293b; border-radius: 12px; padding: 18px 20px; margin-bottom: 24px; text-align: left;">' +
                        '<div style="font-size: 11px; font-weight: 700; color: #38bdf8; text-transform: uppercase; letter-spacing: 0.08em; margin-bottom: 8px;">Action Required</div>' +
                        '<div style="font-size: 14px; color: #e2e8f0; line-height: 1.6;">' +
                            'Please run <strong style="color: #4ade80;">run_server.bat</strong> and open:<br>' +
                            '<a href="' + targetUrl + '" style="color: #38bdf8; text-decoration: underline; font-weight: 600; word-break: break-all; display: inline-block; margin-top: 6px;">' +
                                targetUrl +
                            '</a>' +
                        '</div>' +
                    '</div>' +
                    '<div style="display: flex; gap: 12px; justify-content: center; flex-wrap: wrap;">' +
                        '<a href="' + targetUrl + '" style="display: inline-flex; align-items: center; gap: 8px; background: #16a34a; hover: background: #15803d; color: #ffffff; font-size: 14px; font-weight: 700; padding: 12px 24px; border-radius: 10px; text-decoration: none; box-shadow: 0 4px 14px rgba(22, 163, 74, 0.4);">' +
                            '<span>Open ' + page + ' on Local Server</span> &rarr;' +
                        '</a>' +
                    '</div>' +
                '</div>';

            if (document.body) {
                document.body.innerHTML = '';
                document.body.appendChild(guardContainer);
            } else {
                document.documentElement.appendChild(guardContainer);
            }
        }

        if (document.readyState === 'loading') {
            document.addEventListener('DOMContentLoaded', renderGuard);
        } else {
            renderGuard();
        }

        // Halt further execution in this script thread
        throw new Error('[JEEVAN MARG] file:// protocol blocked. Please start backend server via run_server.bat');
    }
})();
