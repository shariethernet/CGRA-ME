pipeline {
  agent any
  stages {
    stage('Build') {
      steps {
        sh 'rm -rf build'
        echo 'Start build process...'
        timeout(unit: 'MINUTES', time: 10) {
          sh '''ln -snf /var/lib/jenkins/cgrame-resource/gurobi751 ../gurobi751
          mkdir -p build
          cd build
          cmake -DCMAKE_PREFIX_PATH=/home/tech4me/llvm/build -DBUILD_LLVM_PASSES=ON -DCMAKE_BUILD_TYPE=release ../
          make'''
        }
      }
    }
    stage('Test DFG Generation') {
      steps {
        echo 'Testing DFG generation...'
        sh '''export PATH=/home/tech4me/llvm/build/bin/:$PATH
        cd benchmarks/microbench/
        make'''
        sh '''./../../cgrame-resource/microbench_result/diff_microbench_result.sh'''
      }
    }
    stage('Test Mapping') {
      steps {
        echo 'Testing Mapping...'
        sh '''./../../cgrame-resource/mapper_test.sh'''
      }
    }
    stage('Done') {
      steps {
        echo 'Done!'
      }
    }
  }
  post {
    always {
      echo 'Cleaning up...'
      deleteDir()
    }
    success {
      echo 'All Success!'
    }
    failure {
      echo 'Failed!'
    }
  }
}
