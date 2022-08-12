/* global $ */
var login = document.querySelector('.login')
var loginForm = document.querySelector('.login-form')
var register = document.querySelector('.register')
var registerForm = document.querySelector('.register-form')

login.onclick = function () {
	var form = document.getElementById('loginForm');
	form.submit();
}

