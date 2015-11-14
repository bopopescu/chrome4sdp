if (self.importScripts) {
  importScripts('../resources/fetch-test-helpers.js');
}

function consume(reader) {
  var chunks = [];
  function rec(reader) {
    return reader.read().then(function(r) {
        if (r.done) {
          return chunks;
        }
        chunks.push(r.value);
        return rec(reader);
      });
  }
  return rec(reader);
}

function decode(chunks) {
  var decoder = new TextDecoder();
  var result = '';
  for (var chunk of chunks) {
    result += decoder.decode(chunk, {stream: true});
  }
  result += decoder.decode(new Uint8Array(0));
  return result;
}

test(function() {
    var response = new Response(new Blob());
    assert_equals(response.type, 'default',
                  'Default Response.type should be \'default\'');
    assert_equals(response.url, '', 'Response.url should be the empty string');
    assert_equals(response.status, 200,
                  'Default Response.status should be 200');
    assert_true(response.ok, 'Default Response.ok must be true');
    assert_equals(response.statusText, 'OK',
                  'Default Response.statusText should be \'OK\'');
    assert_equals(size(response.headers), 0,
                  'Default Response should not have any header.');

    response.status = 394;
    response.statusText = 'Sesame Street';
    assert_equals(response.status, 200, 'Response.status should be readonly');
    assert_true(response.ok, 'Response.ok must remain unchanged ' +
                             'when Response.status is attempted ' +
                             'unsuccessfully to change');
    assert_equals(response.statusText, 'OK',
                  'Response.statusText should be readonly');
    response.ok = false;
    assert_true(response.ok, 'Response.ok must be readonly');
  }, 'Response default value test');

test(function() {
    var headersInit = new Headers;
    headersInit.set('X-Fetch-Test', 'test');
    var responses =
      [new Response(new Blob(), {status: 303,
                                 statusText: 'tEst',
                                 headers: {'X-Fetch-Test': 'test'}}),
       new Response(new Blob(), {status: 303,
                                 statusText: 'tEst',
                                 headers: [['X-Fetch-Test', 'test']]}),
       new Response(new Blob(), {status: 303,
                                 statusText: 'tEst',
                                 headers: headersInit})];
    responses = responses.concat(
      responses.map(function(r) {return r.clone();}));
    responses.forEach(function(response) {
        assert_equals(response.status, 303, 'Response.status should match');
        assert_false(response.ok, 'Response.ok must be false for 303');
        assert_equals(response.statusText, 'tEst',
                      'Response.statusText should match');
        assert_true(response.headers instanceof Headers,
                    'Response.headers should be Headers');
        assert_equals(size(response.headers), 1,
                      'Response.headers size should match');
        assert_equals(response.headers.get('X-Fetch-Test'),
                      'test',
                      'X-Fetch-Test of Response.headers should match');
      });
  }, 'Response constructor test');

test(function() {
    var response = new Response(new Blob(['dummy'], {type: 'audio/wav'}));
    assert_equals(size(response.headers), 1,
                  'Response.headers should have Content-Type');
    assert_equals(response.headers.get('Content-Type'), 'audio/wav',
                  'Content-Type of Response.headers should be set');

    response = new Response(new Blob(['dummy'], {type: 'audio/wav'}),
                            {
                              headers: {
                                'Content-Type': 'text/html; charset=UTF-8'
                              }
                            });
    assert_equals(size(response.headers), 1,
                  'Response.headers should have Content-Type');
    assert_equals(response.headers.get('Content-Type'),
                  'text/html; charset=UTF-8',
                  'Content-Type of Response.headers should be overridden');

    response = new Response(new Blob(['dummy']));
    assert_equals(size(response.headers), 0,
                  'Response.headers must not have Content-Type ' +
                  'for Blob with type = empty string (1)');

    response = new Response(new Blob(['dummy'], {type: ''}));
    assert_equals(size(response.headers), 0,
                  'Response.headers must not have Content-Type ' +
                  'for Blob with type = empty string (2)');
  }, 'Response content type test');

test(function() {
    [0, 1, 100, 101, 199, 600, 700].forEach(function(status) {
        assert_throws({name: 'RangeError'},
                      function() {
                        new Response(new Blob(), {status: status});
                      },
                      'new Response with status = ' + status +
                      ' should throw');
      });

    [204, 205, 304].forEach(function(status) {
        assert_throws({name: 'TypeError'},
                      function() {
                        new Response(new Blob(), {status: status});
                      },
                      'new Response with null body status = ' + status +
                      ' and body is non-null should throw');
      });

    [300, 0, 304, 305, 306, 309, 500].forEach(function(status) {
        assert_throws({name: 'RangeError'},
                      function() {
                        Response.redirect('https://www.example.com/test.html',
                                          status);
                      },
                      'Response.redirect() with invalid status = ' + status +
                      ' should throw');
      });

    assert_throws(
      {name: 'TypeError'},
      function() {
        Response.redirect('https://', 301);
      },
      'Response.redirect() with invalid URL https:// ' +
      ' and status 301 should throw');

    INVALID_URLS.forEach(function(url) {
        assert_throws(
          {name: 'TypeError'},
          function() {
            Response.redirect(url);
          },
          'Response.redirect() with invalid URL ' + url +
          ' and default status value should throw');
      });

    assert_throws(
      {name: 'TypeError'},
      function() {
        Response.redirect('https://', 300);
      },
      'Response.redirect() with invalid URL https:// ' +
      ' and invalid status 300 should throw TypeError');

    [200, 300, 400, 500, 599].forEach(function(status) {
        var response = new Response(new Blob(), {status: status});
        assert_equals(response.status, status, 'Response.status should match');
        if (200 <= status && status <= 299)
          assert_true(response.ok, 'Response.ok must be true for ' + status);
        else
          assert_false(response.ok, 'Response.ok must be false for ' + status);
      });

    INVALID_HEADER_NAMES.forEach(function(name) {
        assert_throws(
          {name: 'TypeError'},
          function() {
            var obj = {};
            obj[name] = 'a';
            new Response(new Blob(), {headers: obj});
          },
          'new Response with headers with an invalid name (' + name +
          ') should throw');
        assert_throws(
          {name: 'TypeError'},
          function() {
            new Response(new Blob(), {headers: [[name, 'a']]});
          },
          'new Response with headers with an invalid name (' + name +
          ') should throw');
      });
    INVALID_HEADER_VALUES.forEach(function(value) {
        assert_throws(
          {name: 'TypeError'},
          function() {
            new Response(new Blob(),
                         {headers: {'X-Fetch-Test': value}});
          },
          'new Response with headers with an invalid value should throw');
        assert_throws(
          {name: 'TypeError'},
          function() {
            new Response(new Blob(),
                         {headers: [['X-Fetch-Test', value]]});
          },
          'new Response with headers with an invalid value should throw');
      });

    VALID_REASON_PHRASE.forEach(function(text) {
        // new Response() must succeed with a valid statusText.
        var response = new Response(new Blob(), {statusText: text});
        assert_equals(response.statusText, text,
          'Response.statusText must match: ' + text);
      });

    INVALID_REASON_PHRASE.forEach(function(text) {
        assert_throws(
          {name: 'TypeError'},
          function() {
            new Response(new Blob(), {statusText: text});
          },
          'new Response with invalid statusText (' + text +
          ') must throw');
      });
  }, 'Response throw error test');

promise_test(function(t) {
    var res = new Response();
    return consume(res.body.getReader()).then(function(chunks) {
        return decode(chunks);
      }).then(function(text) {
        assert_equals(text, '');
      });
  }, 'Read empty response\'s content');

promise_test(function(t) {
    var res = new Response('hello');
    return consume(res.body.getReader()).then(function(chunks) {
        return decode(chunks);
      }).then(function(text) {
        assert_equals(text, 'hello');
        return res.body.getReader().read();
      }).then(function(r) {
        assert_true(r.done);
        return res.text();
      }).then(function(r) {
        assert_equals(r, '');
      });
  }, 'Read Response body via stream');

promise_test(function(t) {
    var res = new Response('hello');
    res.body.cancel();
    return res.body.getReader().read().then(function(r) {
        assert_true(r.done);
        return res.text();
      }).then(function(r) {
        assert_equals(r, '');
      });
  }, 'Cancel body stream on Response');

promise_test(function(t) {
    // TODO(yhirano): In the current implementation, The body stream always
    // consists of one chunk and hence, we cannot create a meaning test case
    // here. Fix this test case when reading into a size-specified buffer gets
    // possible.
    var size = 8 * 1024 * 1024;
    var buffer = new ArrayBuffer(size);
    var blob = new Blob([buffer]);

    var res = new Response(blob);
    var reader = res.body.getReader();
    var head;
    return reader.read().then(function(r) {
        assert_false(r.done);
        head = r.value;
        assert_not_equals(head.byteLength, 0);
        // TODO(yhirano): See above.
        // assert_not_equals(head.byteLength, size);
        reader.releaseLock();
        return res.arrayBuffer();
      }).then(function(buffer) {
        assert_equals(buffer.byteLength + head.byteLength, size);
        return res.body.getReader().read();
      }).then(function(r) {
        assert_true(r.done);
      });
  }, 'Partial read on Response');

promise_test(function(t) {
    var res = new Response('hello');
    var body = res.body;
    var clone = res.clone();
    assert_not_equals(res.body, body);
    assert_not_equals(res.body, clone.body);
    assert_not_equals(body, clone.body);
    assert_throws({name: 'TypeError'}, function() { body.getReader(); });
    return Promise.all([res.text(), clone.text()]).then(function(r) {
        assert_equals(r[0], 'hello');
        assert_equals(r[1], 'hello');
        return Promise.all([res.text(), clone.text(), res.clone().text()]);
      }).then(function(r) {
        assert_equals(r[0], '');
        assert_equals(r[1], '');
        assert_equals(r[2], '');
      });
  }, 'Clone on Response (text)');

promise_test(function(t) {
    var res = new Response('hello');
    var body = res.body;
    var clone = res.clone();
    assert_not_equals(res.body, body);
    assert_not_equals(res.body, clone.body);
    assert_not_equals(body, clone.body);
    assert_throws({name: 'TypeError'}, function() { body.getReader(); });
    var reader1 = res.body.getReader();
    var reader2 = clone.body.getReader();
    return Promise.all([consume(reader1), consume(reader2)]).then(function(r) {
        assert_equals(decode(r[0]), 'hello');
        assert_equals(decode(r[1]), 'hello');
        return Promise.all([res.text(), clone.text(), res.clone().text()]);
      }).then(function(r) {
        assert_equals(r[0], '');
        assert_equals(r[1], '');
        assert_equals(r[2], '');
      });
  }, 'Clone on Response (manual read)');

promise_test(function(t) {
    var res = new Response('hello');
    var clone = res.clone();
    res.body.cancel();
    return Promise.all([res.text(), clone.text()]).then(function(r) {
        assert_equals(r[0], '');
        assert_equals(r[1], 'hello');
      });
  }, 'Clone and Cancel on Response');

promise_test(function(t) {
    var res = new Response('hello');
    res.body.cancel();
    var clone = res.clone();
    return Promise.all([res.blob(), clone.blob()]).then(function(r) {
        assert_equals(r[0].type, 'text/plain;charset=utf-8', 'cloned type');
        assert_equals(r[1].type, 'text/plain;charset=utf-8', 'original type');
        assert_equals(r[0].size, 0, 'original size');
        assert_equals(r[1].size, 0, 'cloned size');
      });
  }, 'Cancel and Clone on Response');

// Tests for MIME types.
promise_test(function(t) {
    var res = new Response(new Blob(['']));
    return res.blob()
      .then(function(blob) {
          assert_equals(blob.type, '');
          assert_equals(res.headers.get('Content-Type'), null);
        });
  }, 'MIME type for Blob');

promise_test(function(t) {
    var res = new Response(new Blob(['hello'], {type: 'Text/Plain'}));
    return res.blob()
      .then(function(blob) {
          assert_equals(blob.type, 'text/plain');
          assert_equals(blob.size, 5);
          assert_equals(res.headers.get('Content-Type'), 'text/plain');
          return res.blob();
        }).then(function(blob) {
          // When we read from a response twice, it returns an empty contents.
          // But the type should remain.
          assert_equals(blob.type, 'text/plain');
          assert_equals(blob.size, 0);
          assert_equals(res.headers.get('Content-Type'), 'text/plain');
        });
  }, 'MIME type for Blob with non-empty type');

promise_test(function(t) {
    var res = new Response(new FormData());
    return res.blob()
      .then(function(blob) {
          assert_equals(blob.type.indexOf('multipart/form-data; boundary='),
                        0);
          assert_equals(res.headers.get('Content-Type')
                          .indexOf('multipart/form-data; boundary='),
                        0);
        });
  }, 'MIME type for FormData');

promise_test(function(t) {
    var res = new Response('');
    return res.blob()
      .then(function(blob) {
          assert_equals(blob.type, 'text/plain;charset=utf-8');
          assert_equals(res.headers.get('Content-Type'),
                        'text/plain;charset=UTF-8');
        });
  }, 'MIME type for USVString');

promise_test(function(t) {
    var res = new Response(new Blob([''], {type: 'Text/Plain'}),
                           {headers: [['Content-Type', 'Text/Html']]});
    res = res.clone();
    return res.blob()
      .then(function(blob) {
          assert_equals(blob.type, 'text/html');
          assert_equals(res.headers.get('Content-Type'), 'Text/Html');
        });
  }, 'Extract a MIME type with clone');

promise_test(function(t) {
    var res = new Response(new Blob([''], {type: 'Text/Plain'}));
    res.headers.set('Content-Type', 'Text/Html');
    return res.blob()
      .then(function(blob) {
          assert_equals(blob.type, 'text/plain');
          assert_equals(res.headers.get('Content-Type'), 'Text/Html');
        });
  },
  'MIME type unchanged if headers are modified after Response() constructor');

// The following three tests follow different code paths in Body::readAsync().
promise_test(function(t) {
    var res = new Response(new Blob([''], {type: 'Text/Plain'}),
                           {headers: [['Content-Type', 'Text/Html']]});
    return res.blob()
      .then(function(blob) {
          assert_equals(blob.type, 'text/html');
          assert_equals(res.headers.get('Content-Type'), 'Text/Html');
        });
  }, 'Extract a MIME type (1)');

promise_test(function(t) {
    var res = new Response(new Blob([''], {type: 'Text/Plain'}),
                           {headers: [['Content-Type', 'Text/Html']]});
    res.body.cancel();
    return res.blob()
      .then(function(blob) {
          assert_equals(blob.type, 'text/html');
          assert_equals(res.headers.get('Content-Type'), 'Text/Html');
        });
  }, 'Extract a MIME type (2)');

promise_test(function(t) {
    var res = new Response(new Blob([''], {type: 'Text/Plain'}),
                           {headers: [['Content-Type', 'Text/Html']]});
    res.body.cancel();
    res = res.clone();
    return res.blob()
      .then(function(blob) {
          assert_equals(blob.type, 'text/html');
          assert_equals(res.headers.get('Content-Type'), 'Text/Html');
        });
  }, 'Extract a MIME type (3)');

promise_test(function() {
    var response = Response.error();
    return response.text().then(function(text) {
        assert_equals(response.type, 'error');
        assert_equals(response.url, '', 'url must be the empty string');
        assert_equals(response.status, 0, 'status is always 0');
        assert_false(response.ok);
        assert_equals(response.statusText, '',
                      'status message is always the empty byte sequence');
        assert_equals(size(response.headers), 0,
                      'header list is always empty.');
        assert_equals(text, '',
                      'body is always null');
      });
  }, 'Response.error()');

promise_test(function(t) {
    var res = new Response('hello');
    return res.text().then(function(text) {
        assert_equals(text, 'hello');
        return Promise.all([res.text(), res.text()]);
      }).then(function(texts) {
        assert_equals(texts[0], '');
        assert_equals(texts[1], '');
        return res.body.getReader().read();
      }).then(function(r) {
        assert_true(r.done);
        assert_equals(r.value, undefined);
      });
  }, 'Read after text()');

promise_test(function() {
    var response = Response.redirect('https://www.example.com/test.html');
    return response.text().then(function(text) {
        assert_equals(response.status, 302,
                      'default value of status is always 302');
        assert_equals(response.headers.get('location'),
                      'https://www.example.com/test.html',
                      'Location header should be correct absoulte URL');
        assert_throws({name: 'TypeError'},
                      function() {
                        response.headers.append('Accept-Language', 'test');
                      },
                      'response.headers must throw since guard is immutable');
      });
  }, 'Response.redirect() with default status value');

promise_test(function() {
    var response = Response.redirect('https://www.example.com/test.html',
                                     301);
    return response.text().then(function(text) {
        assert_equals(response.status, 301,
                      'value of status is 301');
        assert_equals(response.headers.get('location'),
                      'https://www.example.com/test.html',
                      'Location header should be correct absoulte URL');
        assert_equals(size(response.headers), 1,
                      'Response.redirect().headers must contain ' +
                      'a Location header only');
      });
  }, 'Response.redirect() with 301');

test(function() {
    ['http://ex\x0aample.com',
     'http://ex\x0dample.com'].forEach(function(url) {
        assert_equals(Response.redirect(url).headers.get('Location'),
                      'http://example.com/',
                      'Location header value must not contain CR or LF');
      });
  }, 'Response.redirect() with URLs with CR or LF');

done();
