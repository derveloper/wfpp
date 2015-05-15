var soundboardApp = angular.module('soundboardApp', ['soundboardServices']);

soundboardApp.controller('FileListCtrl', function ($scope, Soundboard) {
    $scope.files = Soundboard.files();
});

var soundboardServices = angular.module('soundboardServices', ['ngResource']);

soundboardServices.factory('Soundboard', ['$resource',
    function($resource){
        return $resource('/api/:endpoint', {}, {
            files: {method:'GET', params:{endpoint:'files'}, isArray: true, transformResponse: function(data, headersGetter) {
                return angular.fromJson(data).files;
            }}
        });
    }]);